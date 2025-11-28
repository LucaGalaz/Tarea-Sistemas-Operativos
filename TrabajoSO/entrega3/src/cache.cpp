#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib> 
#include <algorithm>
#include <chrono>


using namespace std;
using namespace std::chrono;

struct CacheEntry {
    string query;
    string json;
};

static string CACHE_FILE = "cache.db";

CacheEntry parseLine(const string& line){
    size_t pos = line.find('|');
    if (pos == string::npos){
        return{"",""};
    }
    CacheEntry e;
    e.query =line.substr(0,pos);
    e.json = line.substr(pos+1);
    return e;
}

int main(int argc, char* argv[]){
    if (argc != 2){
        cerr << "Uso: " << argv[0] << " \"consulta\"\n";
        return 1;
    }

    auto tInicioTotal = steady_clock::now();

    // Tamaño de cache
    int CACHE_SIZE = 3;
    if (const char* p = getenv("CACHE_SIZE")){
        try { CACHE_SIZE = max(1, stoi(p)); } catch(...) {}
    }

    // la consulta que viene del buscador (ruta_idx;palabra)
    string fullQuery = argv[1];
    string query = fullQuery;

    // extraer solo la palabra para mostrarla en el json
    string queryPalabra = fullQuery;
    size_t pos = fullQuery.find(';');
    if (pos != string::npos)
        queryPalabra = fullQuery.substr(pos + 1);

    // lee topk para mostrarlo en el json
    int TOPK = 0;
    if (const char* pTop = getenv("TOPK")){
        try { TOPK = max(1, stoi(pTop)); } catch(...) {}
    }

    // carga cache desde archivo
    vector<CacheEntry> cache;
    {
        ifstream in(CACHE_FILE);
        string line;
        while (getline(in, line)){
            if (line.empty()) continue;
            CacheEntry e = parseLine(line);
            if (!e.query.empty())
                cache.push_back(e);
        }
    }

    // busca en la cache
    for (const auto& e : cache){
        if (e.query == query){
            auto tFinTotal = steady_clock::now();
            long tiempoCacheMs = duration_cast<milliseconds>(tFinTotal - tInicioTotal).count();
            long tiempoMotorMs = 0;

            cout << "{\n"
                 << "  \"query\": \"" << queryPalabra << "\",\n"
                 << "  \"Origen respuesta\": \"cache\",\n"
                 << "  \"tiempo cache\": " << tiempoCacheMs << ",\n"
                 << "  \"tiempo motorB\": " << tiempoMotorMs << ",\n"
                 << "  \"Tiempo total\": " << tiempoCacheMs << ",\n"
                 << "  \"topk\": " << TOPK << ",\n"
                 << "  \"respuesta\": " << e.json << "\n"
                 << "}\n";

            return 0;
        }
    }

    // llamar al motor
    const char* motorPath = getenv("MOTOR_BUSQUEDA");
    if (!motorPath){
        cerr << "ERROR: variable de entorno MOTOR_BUSQUEDA no definida.\n";
        return 1;
    }

    auto tAntesMotor = steady_clock::now();

    string tmpFile = "./tmp_response.json";
    ostringstream cmd;
    cmd << motorPath << " \"" << query << "\" > " << tmpFile;

    int ret = system(cmd.str().c_str());
    if (ret != 0){
        cerr << "ERROR: fallo al ejecutar el MOTOR_DE_BUSQUEDA.\n";
        return 1;
    }

    auto tDespuesMotor = steady_clock::now();
    long tiempoMotorMs = duration_cast<milliseconds>(tDespuesMotor - tAntesMotor).count();

    string jsonResponse;
    {
        ifstream in(tmpFile);
        if (!in){
            cerr << "ERROR: no se pudo leer " << tmpFile << "\n";
            return 1;
        }
        ostringstream buf;
        buf << in.rdbuf();
        jsonResponse = buf.str();
    }

    cache.push_back({query, jsonResponse});

    if ((int)cache.size() > CACHE_SIZE){
        cache.erase(cache.begin()); // eliminar el mas antiguo
    }

    {
        ofstream out(CACHE_FILE, ios::trunc);
        for (auto& e : cache){
            out << e.query << "|" << e.json << "\n";
        }
    }

    auto tFinTotal = steady_clock::now();
    long tiempoTotalMs = duration_cast<milliseconds>(tFinTotal - tInicioTotal).count();

    // ---- imprime respuesta final 
    cout << "{\n"
         << "  \"query\": \"" << queryPalabra << "\",\n"
         << "  \"Origen respuesta\": \"motor b\",\n"
         << "  \"tiempo cache\": 0,\n"
         << "  \"tiempo motorB\": " << tiempoMotorMs << ",\n"
         << "  \"Tiempo total\": " << tiempoTotalMs << ",\n"
         << "  \"topk\": " << TOPK << ",\n"
         << "  \"respuesta\": " << jsonResponse << "\n"
         << "}\n";

    return 0;
}