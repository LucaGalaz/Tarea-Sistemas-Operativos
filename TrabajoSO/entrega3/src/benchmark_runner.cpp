#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <unistd.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

int main() {
    // 1. Configuración y validación de Entorno
    const char* indexadorProg = getenv("INDICE_INVET_PARALELO");
    const char* plotScript = getenv("PLOT_SCRIPT");             // Ruta a plot_performance.py
    const char* statsFolder = getenv("STATS_FOLDER");           // Carpeta para guardar gráfico y log

    if (!indexadorProg || !plotScript || !statsFolder) {
        cerr << "Error: Faltan variables de entorno (INDICE_INVET_PARALELO, PLOT_SCRIPT o STATS_FOLDER).\n";
        return 1;
    }

    // Asegurar que exista la carpeta de estadisticas
    if (!fs::exists(statsFolder)) {
        fs::create_directory(statsFolder);
    }

    string logPath = string(statsFolder) + "/benchmark_data.csv";
    
    // 2. Definir Arreglo de Threads (Requerimiento )
    // Probamos con 1 hasta hardware_concurrency + unos extra para ver saturación
    vector<int> CANT_THREADS = {1, 2, 3, 4, 5, 6, 8, 12, 16};

    cout << "--- Iniciando Benchmark de Rendimiento ---\n";
    cout << "Guardando datos en: " << logPath << "\n";

    ofstream logFile(logPath);
    logFile << "threads,tiempo_ms\n"; // Cabecera CSV

    string idxTemp = string(statsFolder) + "/temp_bench.idx";
    string librosDir = "./Libros"; // Asumimos ruta relativa o usar variable de entorno

    // 3. Loop de ejecución (Requerimiento )
    for (int n : CANT_THREADS) {
        cout << "Ejecutando con N_THREADS = " << n << "... " << flush;

        // Configurar variable de entorno para el proceso hijo
        string envVar = to_string(n);
        setenv("N_THREADS", envVar.c_str(), 1);

        // Preparar comando: programa salida carpeta
        // Redirigimos cout a null para no ensuciar la pantalla, pero dejamos cerr
        string comando = string(indexadorProg) + " \"" + idxTemp + "\" \"" + librosDir + "\" > /dev/null";

        // 4. Medición de tiempo (Requerimiento )
        auto start = chrono::high_resolution_clock::now();
        
        int ret = system(comando.c_str());
        
        auto end = chrono::high_resolution_clock::now();

        if (ret != 0) {
            cerr << "[FALLO]\n";
            continue; 
        }

        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "Tiempo: " << duration << " ms\n";

        // Registrar en log
        logFile << n << "," << duration << "\n";
    }
    logFile.close();

    // 5. Llamar a Python para graficar (Requerimiento [cite: 45, 51])
    cout << "Generando gráfico con Python...\n";
    string cmdPython = "python3 \"" + string(plotScript) + "\" \"" + logPath + "\" \"" + string(statsFolder) + "\"";
    system(cmdPython.c_str());

    cout << "Proceso finalizado. Revise la carpeta: " << statsFolder << "\n";
    
    // Limpieza archivo temporal
    if (fs::exists(idxTemp)) fs::remove(idxTemp);

    return 0;
}