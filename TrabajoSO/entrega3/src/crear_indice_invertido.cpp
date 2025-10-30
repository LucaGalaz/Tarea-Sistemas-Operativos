#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <cctype>
#include <sstream>
#include <unistd.h>

using namespace std;
namespace fs = std::filesystem;

string limpiarPalabra(const string &pal) {
    string out;
    for (char c : pal) {
        if (isalpha(c)) out += tolower(c);
    }
    return out;
}

int main(int argc, char* argv[]) {
    cout << "PID de crear_indice_invertido: " << getpid() << "\n";

    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <archivo_salida.idx> <carpeta_libros>\n";
        return 1;
    }

    string archivoSalida = argv[1];
    string carpetaLibros = argv[2];

    fs::path ruta_carpeta(carpetaLibros);
    if (!fs::exists(ruta_carpeta) || !fs::is_directory(ruta_carpeta)) {
        cerr << "Error: '" << carpetaLibros << "' no existe o no es un directorio.\n";
        return 1;
    }

    map<string, map<string,int>> indice;

    for (auto &entrada : fs::directory_iterator(carpetaLibros)) {
        if (entrada.is_regular_file()) {
            string nombreLibro = entrada.path().filename().string();
            ifstream archivoEntrada(entrada.path());
            if (!archivoEntrada) continue;

            string palabra;
            while (archivoEntrada >> palabra) {
                string limpia = limpiarPalabra(palabra);
                if (!limpia.empty()) {
                    indice[limpia][nombreLibro]++;
                }
            }
        }
    }

    ofstream temporal("tmp_indice.txt");
    for (auto &par : indice) {
        temporal << par.first;
        for (auto &doc : par.second) {
            temporal << ";(\"" << doc.first << "\"," << doc.second << ")";
        }
        temporal << "\n";
    }
    temporal.close();

    const char* programaIdx = getenv("CREATE_IDX");
    if (programaIdx == nullptr) {
        cerr << "La variable de entorno CREATE_IDX no esta definida.\n";
    } else {
        string comando = string(programaIdx) + " \"" + archivoSalida + "\" \"tmp_indice.txt\"";
        cout << "Ejecutando: " << comando << "\n";
        system(comando.c_str());
    }

    cout << "Para volver al menu escriba '0'...\n";
    int opcion;
    do {
        cin >> opcion;
    } while (opcion != 0);

    return 0;
}