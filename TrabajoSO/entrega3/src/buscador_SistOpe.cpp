#include <iostream>
#include <string>
#include <cstdlib> 
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h> 

using namespace std;

// funcion para limpiar la palabra de búsqueda 
string limpiarPalabra(const string& query) {
    string cleaned;
    for (char c : query) {
        if (isalpha(static_cast<unsigned char>(c))) {
            cleaned += tolower(c);
        }
    }
    return cleaned;
}

// funcion para obtener la palabra de busqueda
string obtenerPalabra() {
    string palabra;
    cout << "Ingrese la palabra a buscar: ";
    getline(cin, palabra); 
    return limpiarPalabra(palabra);
}

int main(int argc, char* argv[]) {
    // obtener y mostrar pid
    cerr << "PID del BUSCADOR SistOpe: " << getpid() << endl;

    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <ruta_archivo.idx>\n";
        return 1;
    }

    string filePath = argv[1];
    
    // obtener la palabra a buscar del usuario (ya limpia)
    string palabraBuscada = obtenerPalabra();

    if (palabraBuscada.empty()) {
        cout << "Búsqueda cancelada: palabra vacía o inválida.\n";
        return 0;
    }

    // obtener la ruta del ejecutable de la caché
    string cachePath = "bin/cache"; 
    
    // Componer la consulta para la caché: "ruta_idx;palabra"
    string consulta = filePath + ";" + palabraBuscada;
    
    ostringstream cmd;
    cmd << cachePath << " \"" << consulta << "\"";

    cout << "\n>>> Resultados de Búsqueda <<<\n";
    cout << "Índice usado: " << filePath << "\n";
    
    int ret = system(cmd.str().c_str());
    
    if (ret != 0) {
        cerr << "Error: El proceso de caché falló con código " << ret << ".\n";
        return 1;
    }

    cout << "\n-----------------------------\n";
    return 0;
}