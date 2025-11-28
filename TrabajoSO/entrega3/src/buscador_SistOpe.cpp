#include <iostream>
#include <string>
#include <cstdlib> // Para getenv() y system()
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h> // Para getpid()

using namespace std;

// Función para limpiar la palabra de búsqueda (solo alfabéticos y minúsculas)
string limpiarPalabra(const string& query) {
    string cleaned;
    for (char c : query) {
        if (isalpha(static_cast<unsigned char>(c))) {
            cleaned += tolower(c);
        }
    }
    return cleaned;
}

// Función para obtener la palabra de búsqueda
string obtenerPalabra() {
    string palabra;
    cout << "Ingrese la palabra a buscar: ";
    getline(cin, palabra); 
    return limpiarPalabra(palabra);
}

int main(int argc, char* argv[]) {
    // 1. OBTENER Y MOSTRAR PID (Requisito 5)
    cerr << "PID del BUSCADOR SistOpe: " << getpid() << endl;

    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <ruta_archivo.idx>\n";
        return 1;
    }

    string filePath = argv[1];
    
    // 2. Obtener la palabra a buscar del usuario (ya limpia)
    string palabraBuscada = obtenerPalabra();

    if (palabraBuscada.empty()) {
        cout << "Búsqueda cancelada: palabra vacía o inválida.\n";
        return 0;
    }

    // 3. Obtener la ruta del ejecutable de la caché
    // Asumimos que el ejecutable se llama 'cache' y está en 'bin/'
    string cachePath = "bin/cache"; 
    
    // 4. Componer la consulta para la caché: "ruta_idx;palabra"
    string consulta = filePath + ";" + palabraBuscada;
    
    // 5. Ejecutar el módulo de caché con la consulta compuesta
    ostringstream cmd;
    cmd << cachePath << " \"" << consulta << "\"";

    cout << "\n>>> Resultados de Búsqueda <<<\n";
    cout << "Palabra buscada: '" << palabraBuscada << "'\n";
    cout << "Índice usado: " << filePath << "\n";
    
    // La salida JSON ya tiene el formato correcto {"Respuesta":[{"Libro":"...","score":...}]}
    // Usamos system() para ejecutar cache y que su salida (el JSON) se imprima directamente.
    int ret = system(cmd.str().c_str());
    
    if (ret != 0) {
        cerr << "Error: El proceso de caché falló con código " << ret << ".\n";
        return 1;
    }

    cout << "\n-----------------------------\n";
    return 0;
}