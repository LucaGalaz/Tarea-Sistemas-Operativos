#include <iostream>
#include <fstream> 
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib> 
#include <algorithm> 
#include <cmath>
#include <stdexcept>

using namespace std;

// Estructura para almacenar un resultado de búsqueda
struct Resultado {
    string nombre_libro; // mombre del archivo usado como "Libro" en la salida
    double score; // frecuencia de la palabra en el documento
};

// comparador para ordenar los resultados de mayor a menor score
bool compararResultados(const Resultado& a, const Resultado& b) {
    return a.score > b.score;
}

// funcion auxiliar para extraer el nombre y frecuencia de una subcadena del indice secuencial
bool parsearEntradaIndiceSecuencial(const string& entrada, string& nombreLibro, int& frecuencia) {
    
    size_t startName = entrada.find('"');
    size_t endName = entrada.find('"', startName + 1);
    size_t startFreq = entrada.find(',', endName + 1);
    size_t endFreq = entrada.find(')', startFreq + 1);
    
    if (startName == string::npos || endName == string::npos || 
        startFreq == string::npos || endFreq == string::npos) {
        return false;
    }

    try {
        // extrae el nombre del libro 
        nombreLibro = entrada.substr(startName + 1, endName - startName - 1);
        // Extrae la frecuencia
        string freqStr = entrada.substr(startFreq + 1, endFreq - startFreq - 1);
        frecuencia = stoi(freqStr);
        return true;
    } catch (const exception& e) {
        return false;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " \"consulta\"\n";
        return 1;
    }
    string fullQuery = argv[1];

    //obtiene el TOPK desde la variable de entorno
    int TOPK = 3; 
    if (const char* p = getenv("TOPK")) {
        try {
            TOPK = max(1, stoi(p));
        } catch(...) {
            cerr << "Advertencia: TOPK de entorno invalido, usando valor por defecto: " << TOPK << endl;
        }
    }
    
    //Descompone la consulta de cache: "ruta_idx;palabra"
    size_t sepPos = fullQuery.find(';');
    if (sepPos == string::npos) {
        cerr << "[MOTOR] Error: Formato de consulta incorrecto. Esperado: 'ruta_idx;palabra'\n";
        return 1;
    }
    
    string filePath = fullQuery.substr(0, sepPos);
    string palabraBuscada = fullQuery.substr(sepPos + 1);
    
    vector<Resultado> resultadosReales;
    
    ifstream idxFile(filePath); 
    if (!idxFile.is_open()) { 
        cerr << "[MOTOR] Error: No se pudo abrir el archivo de indice: " << filePath << endl;
        // Respuesta en formato JSON solicitado por el profesor, sin resultados
        cout << "{\"Respuesta\":[]}" << endl;
        return 0; 
    }

    string linea;
    while (getline(idxFile, linea)) {
        size_t pos = linea.find(';');

        //Obtiene la palabra de la linea y la compara
        string palabraIdx = (pos == string::npos) ? linea : linea.substr(0, pos);
        
        if (palabraIdx != palabraBuscada) {
            continue;
        }
        
        string listaDocs = (pos == string::npos) ? "" : linea.substr(pos + 1);
        if (listaDocs.empty()) continue;

        stringstream ss(listaDocs);
        string entradaDoc;
        
        while (getline(ss, entradaDoc, ';')) {
            if (entradaDoc.empty()) continue;

            string nombreLibro;
            int frecuencia = 0;

            if (parsearEntradaIndiceSecuencial(entradaDoc, nombreLibro, frecuencia)) {
                if (frecuencia > 0) {
                    resultadosReales.push_back({nombreLibro, (double)frecuencia});
                }
            } else {
            }
        }
    }
    idxFile.close();
    
    sort(resultadosReales.begin(), resultadosReales.end(), compararResultados);

    // selecciona solo los TOPK mejores resultados
    size_t numResultados = min((size_t)TOPK, resultadosReales.size());

    stringstream jsonOutput;
    
    jsonOutput << "{\"Respuesta\":[";

    for (size_t i = 0; i < numResultados; ++i) {
        const auto& res = resultadosReales[i];
        
        // Formato: {"Libro":"nombre","score":3}
        // El score se imprime como entero
        jsonOutput << "{\"Libro\":\"" << res.nombre_libro << "\",\"score\":" << (int)res.score << "}";
        
        if (i < numResultados - 1) {
            jsonOutput << ","; 
        }
    }

    jsonOutput << "]}"; 
    
    cout << jsonOutput.str() << endl; 

    return 0;
}