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

static string LIBROS_DIR="./Libros"; //carpeta libros
static string LOG_PATH="./logs/idx_parallel.log"; //archivo de log
static string MAPA_PATH="./Libros/MAPA-LIBROS.txt"; //mapa de libros

static mutex mtx_idx, mtx_log; //mutex para indice y log

static map<string,map<int,int>> INDICE; //indice global: palabra -> libroID -> cantidad

struct Libro { int id; string nombre; fs::path path; };

//limpia palabra: solo alfanumericos y minusculas
string limpiarPalabra(const string &pal){
    string out;
    for(char c:pal) if(isalnum(c)) out+=tolower(c);
    return out;
}

static inline string ts_now(){
    using namespace chrono;
    auto t=system_clock::now();
    time_t tt=system_clock::to_time_t(t);
    tm tmv{};
    #ifdef _WIN32
    localtime_s(&tmv,&tt);
    #else
    localtime_r(&tt,&tmv);
    #endif
    char buf[32]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&tmv);
    return string(buf);
}

//asegura existencia de ./logs y ./obj
static void asegurarDirs(){
    if(!fs::exists("./logs")) fs::create_directory("./logs");
    if(!fs::exists("./obj")) fs::create_directory("./obj");
}

//construye mapa de libros: vector con id y nombre
static vector<Libro> construirMapaLibros(const string& dir){
    vector<Libro> L;
    for(auto &e: fs::directory_iterator(dir)){
        if(!e.is_regular_file()) continue;
        auto fname=e.path().filename().string();
        if(fname=="MAPA-LIBROS.txt") continue; 
        L.push_back({0,fname,e.path()});
    }
    sort(L.begin(),L.end(),[](auto &a,auto &b){ return a.nombre<b.nombre; }); //orden alfabetico
    for(int i=0;i<(int)L.size();++i) L[i].id=i+1; //asigna ID
    ofstream mapa(MAPA_PATH,ios::trunc);
    for(auto &x:L) mapa<<x.id<<"; "<<x.nombre<<"\n";
    return L; //retorna vector con libros
}

//procesa un libro: cuenta palabras, fusiona al indice global y escribe log
static void procesarLibro(const Libro &L,int idThread){
    string t0=ts_now();
    unordered_map<string,int> local; //indice local
    ifstream in(L.path);
    string tok;
    while(in>>tok){
        tok=limpiarPalabra(tok);
        if(!tok.empty()) local[tok]++;
    }
    int total=0; for(auto &kv:local) total+=kv.second;

    { //fusion al indice global
    lock_guard<mutex> lk(mtx_idx);
    for(auto &kv:local) INDICE[kv.first][L.id]+=kv.second;
    }

    string t1=ts_now();

    { //escribir log
    lock_guard<mutex> lk(mtx_log);
    ofstream lg(LOG_PATH,ios::app);
    lg<<idThread<<";"<<L.id<<";"<<total<<";"<<t0<<";"<<t1<<"\n";
    }
}

int main(int argc,char* argv[]){
    if(argc!=3){ cerr<<"Uso: "<<argv[0]<<" <salida.idx> <carpeta_libros>\n"; return 1;}
    string salidaIdx=argv[1];
    LIBROS_DIR=argv[2];

    asegurarDirs(); //crear dirs si no existen

    if(!fs::exists(LIBROS_DIR)||!fs::is_directory(LIBROS_DIR)){
        cerr<<"Error: '"<<LIBROS_DIR<<"' no existe o no es directorio.\n";
        return 1;
    }

    int NTHREADS=(int)thread::hardware_concurrency(); 
    if(NTHREADS<=0) NTHREADS=2;
    if(const char* p=getenv("N_THREADS")) try{ NTHREADS=max(1,stoi(p)); } catch(...){}

    int NLOTE=4; //tamaño de lote por defecto
    if(const char* p=getenv("N_LOTE")) try{ NLOTE=max(1,stoi(p)); } catch(...){}

    vector<Libro> libros=construirMapaLibros(LIBROS_DIR);
    if(libros.empty()){ cerr<<"No se encontraron libros en: "<<LIBROS_DIR<<"\n"; return 1;}

    ofstream lg(LOG_PATH,ios::trunc);
    lg<<"id_thread;id_libro;cant_palabras;ts_inicio;ts_termino\n";

    const int n=(int)libros.size();
    for(int ini=0;ini<n;ini+=NLOTE){
        int fin=min(ini+NLOTE,n);
        atomic<int> next{ini};
        int H=min(NTHREADS,fin-ini);
        vector<thread> pool; pool.reserve(H);

        auto worker=[&](int idThread){
            while(true){
                int k=next.fetch_add(1);
                if(k>=fin) break;
                procesarLibro(libros[k],idThread);
            }
        };

        for(int t=1;t<=H;++t) pool.emplace_back(worker,t);
        for(auto &th:pool) th.join();
    }

    { //.idx final
        ofstream out(salidaIdx,ios::trunc);
        for(auto &p:INDICE)
            for(auto &doc:p.second) out<<p.first<<";"<<doc.first<<";"<<doc.second<<"\n";
    }

    cout<<"OK\n";
    cout<<"  IDX:  "<<salidaIdx<<"\n";
    cout<<"  MAPA: "<<MAPA_PATH<<"\n";
    cout<<"  LOG:  "<<LOG_PATH<<"\n";
    return 0;
}
