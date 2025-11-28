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
    string nombre_libro; // Nombre del archivo, usado como "Libro" en la salida
    double score; // Frecuencia de la palabra en el documento
};

// Comparador para ordenar los resultados de mayor a menor score
bool compararResultados(const Resultado& a, const Resultado& b) {
    return a.score > b.score;
}

// Función auxiliar para extraer el nombre y frecuencia de una subcadena del indice secuencial: ("\"nombre\",3)"
bool parsearEntradaIndiceSecuencial(const string& entrada, string& nombreLibro, int& frecuencia) {
    // La entrada debe ser algo como: ("libro_1399.txt",3)
    
    // 1. Busca la primera comilla doble para el nombre
    size_t startName = entrada.find('"');
    // 2. Busca la segunda comilla doble
    size_t endName = entrada.find('"', startName + 1);
    // 3. Busca la coma antes de la frecuencia
    size_t startFreq = entrada.find(',', endName + 1);
    // 4. Busca el paréntesis de cierre
    size_t endFreq = entrada.find(')', startFreq + 1);
    
    if (startName == string::npos || endName == string::npos || 
        startFreq == string::npos || endFreq == string::npos) {
        return false;
    }

    try {
        // Extrae el nombre del libro (entre comillas)
        nombreLibro = entrada.substr(startName + 1, endName - startName - 1);
        // Extrae la frecuencia (después de la coma y hasta el paréntesis)
        string freqStr = entrada.substr(startFreq + 1, endFreq - startFreq - 1);
        frecuencia = stoi(freqStr);
        return true;
    } catch (const exception& e) {
        // En caso de error de conversión (stoi), retorna falso
        return false;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " \"consulta\"\n";
        return 1;
    }
    string fullQuery = argv[1];
    cerr << "[MOTOR] llamado con consulta: " << fullQuery << endl;

    // 1. Obtener TOPK desde la variable de entorno
    int TOPK = 3; 
    if (const char* p = getenv("TOPK")) {
        try {
            TOPK = max(1, stoi(p));
        } catch(...) {
            cerr << "Advertencia: TOPK de entorno invalido, usando valor por defecto: " << TOPK << endl;
        }
    }
    cerr << "[MOTOR] Usando TOPK = " << TOPK << endl;
    
    // 2. Descomponer la consulta de cache: "ruta_idx;palabra"
    size_t sepPos = fullQuery.find(';');
    if (sepPos == string::npos) {
        cerr << "[MOTOR] Error: Formato de consulta incorrecto. Esperado: 'ruta_idx;palabra'\n";
        return 1;
    }
    
    string filePath = fullQuery.substr(0, sepPos);
    string palabraBuscada = fullQuery.substr(sepPos + 1);

    cerr << "[MOTOR] Buscando palabra '" << palabraBuscada << "' en indice: " << filePath << endl;
    
    vector<Resultado> resultadosReales;
    
    // 3. Cargar/Leer el Índice e identificar resultados
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

        // 3.1. Obtener la palabra de la linea y compararla
        string palabraIdx = (pos == string::npos) ? linea : linea.substr(0, pos);
        
        // La limpieza en el buscador debe asegurar que esta comparación funcione
        if (palabraIdx != palabraBuscada) {
            continue;
        }
        
        // 3.2. Procesar la lista de documentos para la palabra encontrada
        // La listaDocs es el resto de la linea (Ej: ("libro_1.txt",5);("libro_2.txt",2))
        string listaDocs = (pos == string::npos) ? "" : linea.substr(pos + 1);
        if (listaDocs.empty()) continue;

        // Utilizamos stringstream para dividir los documentos por el separador ";"
        stringstream ss(listaDocs);
        string entradaDoc;
        
        // El resto de la línea son entradas separadas por ;
        while (getline(ss, entradaDoc, ';')) {
            if (entradaDoc.empty()) continue;

            string nombreLibro;
            int frecuencia = 0;

            // La función de parseo extrae el nombre y la frecuencia
            if (parsearEntradaIndiceSecuencial(entradaDoc, nombreLibro, frecuencia)) {
                if (frecuencia > 0) {
                    resultadosReales.push_back({nombreLibro, (double)frecuencia});
                }
            } else {
                 // Si el parseo falla, se ignora esa sub-entrada y se pasa a la siguiente
            }
        }
    }
    idxFile.close();
    
    // 4. Aplicar el algoritmo TOPK: Ordenar y seleccionar
    sort(resultadosReales.begin(), resultadosReales.end(), compararResultados);

    // Seleccionar solo los TOPK mejores resultados
    size_t numResultados = min((size_t)TOPK, resultadosReales.size());

    // 5. Generar la respuesta JSON con la nueva estructura en español
    stringstream jsonOutput;
    
    // Inicio del objeto JSON principal
    jsonOutput << "{\"Respuesta\":[";

    for (size_t i = 0; i < numResultados; ++i) {
        const auto& res = resultadosReales[i];
        
        // Formato: {"Libro":"nombre","score":3}
        // El score se imprime como entero ya que es una frecuencia.
        jsonOutput << "{\"Libro\":\"" << res.nombre_libro << "\",\"score\":" << (int)res.score << "}";
        
        if (i < numResultados - 1) {
            jsonOutput << ","; 
        }
    }

    jsonOutput << "]}"; // Cierre del array "Respuesta" y del objeto principal
    
    cout << jsonOutput.str() << endl; 

    return 0;
}