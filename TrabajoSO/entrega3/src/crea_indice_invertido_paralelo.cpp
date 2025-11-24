#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <cctype>
#include <sstream>
#include <unistd.h>
#include <map>

using namespace std;
namespace fs = std::filesystem;

static string LIBROS_DIR = "./Libros";
static string LOG_PATH   = "./logs/idx_parallel.log";
static string MAPA_PATH  = "./Libros/MAPA-LIBROS.txt";

static mutex mtx_idx, mtx_log;

//indice guarda de la siguiente manera (string es el nombre, el primer int es el id del libro y el segundo es la cantidad de veces que aparece)
static map<string, map<int,int>> INDICE;

struct Libro { int id; string nombre; fs::path path; };


string limpiarPalabra(const string &pal) {
    string out;
    for (char c : pal) {
        if (isalpha(c)) out += tolower(c); 
    }
    return out;
}

//retorna un string con el fecha y hora para timestamp de inicio y timestamp de término 
static inline string ts_now(){
    using namespace std::chrono;
    auto t  = system_clock::now();
    time_t tt = system_clock::to_time_t(t);
    tm tmv{};
    #ifdef _WIN32
    localtime_s(&tmv, &tt);
    #else
    localtime_r(&tt, &tmv);
    #endif
    char buf[32]; strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmv);
    return string(buf);
}

//asegura que existann ./logs y ./obj
static void asegurarDirs(){
    if (!fs::exists("./logs")) fs::create_directory("./logs");
    if (!fs::exists("./obj"))  fs::create_directory("./obj");
}

//Cronstruimos mapa de libros
static vector<Libro> construirMapaLibros(const string& dir){
    vector<Libro> L;
    //Recorremos la carpeta libros
    for (auto &e : fs::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        auto fname = e.path().filename().string();
        //evitamos que se incluya MAPA-LIBROS.txt
        if (fname == "MAPA-LIBROS.txt") continue;
        //guarda en el vector
        L.push_back({0, fname, e.path()});
    }
    //ordena alfabeticamente los nombres
    sort(L.begin(), L.end(), [](auto& a, auto& b){ return a.nombre < b.nombre; });
    //asigna los ID
    for (int i=0;i<(int)L.size();++i) 
        L[i].id = i+1;

    //crea el archivo MAPA-LIBROS.txt
    ofstream mapa(MAPA_PATH, ios::trunc);
    for (auto &x : L) 
        mapa << x.id << "; " << x.nombre << "\n";

    //retornamos el vector con todos los libros
    return L;
}

//procesa un libro cuenta palabra por palabra, fusiona al indice global y deja linea en el log 
static void procesarLibro(const Libro& L, int idThread) {
    string t0 = ts_now();
    // conteo local
    unordered_map<string,int> local;
    ifstream in(L.path);
    string tok;
    while (in >> tok) {
        tok = limpiarPalabra(tok);
        if (!tok.empty() && tok.size() > 1) {  // solo palabras de 2 o más letras
            local[tok]++;
        }
    }
    int total = 0; for (auto& kv : local) total += kv.second;

    //fusion  al INDICE
    {
    lock_guard<mutex> lk(mtx_idx); //se ocupa el lock_guard<mutex> para que solo un hilo a la vez tenga el permiso de modifcar 
    for (auto& kv : local) {       //el indice globlal y asi evitar errores de acceso simultaneo
        INDICE[kv.first][L.id] += kv.second; 
    }
    }

    //toma de tiempo
    string t1 = ts_now();

    // escribe el log
    {
    lock_guard<mutex> lk(mtx_log);
    ofstream lg(LOG_PATH, ios::app);
    lg << idThread << ";" << L.id << ";" << total << ";" << t0 << ";" << t1 << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <salida.idx> <carpeta_libros>\n";
        return 1;
    }
    string salidaIdx = argv[1];
    LIBROS_DIR = argv[2];

    asegurarDirs();

    // validar carpeta de los libros
    if (!fs::exists(LIBROS_DIR) || !fs::is_directory(LIBROS_DIR)) {
        cerr << "Error: '" << LIBROS_DIR << "' no existe o no es directorio.\n";
        return 1;
    }

    //leemos las variables de entorno N-THREADS y N-LOTE
    int NTHREADS = (int)thread::hardware_concurrency();
    if (NTHREADS <= 0) NTHREADS = 2;
    if (const char* p = getenv("N_THREADS")) { try { NTHREADS = max(1, stoi(p)); } catch(...){} }

    int NLOTE = 4;
    if (const char* p = getenv("N_LOTE")) { try { NLOTE = max(1, stoi(p)); } catch(...){} }

    // construimos mapa de libros y lo guardamos en MAPA-LIBROS.txt
    vector<Libro> libros = construirMapaLibros(LIBROS_DIR);
    if (libros.empty()) {
        cerr << "No se encontraron libros en: " << LIBROS_DIR << "\n";
        return 1;
    }

    //encabezado del log
    ofstream lg(LOG_PATH, ios::trunc);
    lg << "id_thread;id_libro;cant_palabras;ts_inicio;ts_termino\n";

    // procesamiento por lotes + threads
    const int n = (int)libros.size();
    for (int ini = 0; ini < n; ini += NLOTE) {
        int fin = min(ini + NLOTE, n);
        atomic<int> next{ini};
        int H = min(NTHREADS, fin - ini);

        vector<thread> pool; pool.reserve(H);
        auto worker = [&](int idThread) {
            while (true) {
                int k = next.fetch_add(1);
                if (k >= fin) break;
                procesarLibro(libros[k], idThread);
            }
        };

        for (int t = 1; t <= H; ++t) pool.emplace_back(worker, t);
        for (auto& th : pool) th.join();
    }

    // volcar .idx final con ID en vez de nombre: palabra;idLibro;frecuencia
    {
        ofstream out(salidaIdx, ios::trunc);
        for (auto& p : INDICE) {
            for (auto& doc : p.second) {
                out << p.first << ";" << doc.first << ";" << doc.second << "\n";
            }
        }
    }

    // resumen
    cout << "OK\n";
    cout << "  IDX:  " << salidaIdx << "\n";
    cout << "  MAPA: " << MAPA_PATH << "\n";
    cout << "  LOG:  " << LOG_PATH << "\n";
    return 0;
    
}