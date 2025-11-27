#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib> 
#include <algorithm>

using namespace std;

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
    if (argc !=2 ){
        cerr<<"Uso: " << argv[0] << " \"consulta\"\n";
        return 1;
    }
    int CACHE_SIZE = 3;
    string query= argv[1];
    if (const char* p = getenv("CACHE_SIZE")) {
        try {
            CACHE_SIZE = max(1, stoi(p));
        } catch(...) {}
    }
    //cargo cache desde archivo
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
    //buscador 
    for (const auto& e : cache){
        if (e.query == query){
            //encontro la consulta en el cache
            cout<<e.json<< "\n";
            return 0;
        }
    }

    const char* motorPath=getenv("MOTOR_BUSQUEDA");
    if (!motorPath) {
        cerr << "ERROR: variable de entorno MOTOR_BUSQUEDA no definida.\n";
        return 1;
    }

    //archivo temporal para capturar la respuesta del motor
    string tmpFile = "./tmp_response.json";
    ostringstream cmd;
    cmd << motorPath << " \"" << query << "\" > " << tmpFile;

    int ret = system(cmd.str().c_str());
    if (ret != 0) {
        cerr << "ERROR: fallo al ejecutar el MOTOR_DE_BUSQUEDA.\n";
        return 1;
    }

    //Leer la respuesta JSON del archivo temporal
    string jsonResponse;
    {
        ifstream in(tmpFile);
        if(!in){
            cerr << "ERROR: no se pudo leer " << tmpFile <<"\n";
            return 1; 
        }
        ostringstream buf;
        buf << in.rdbuf();
        jsonResponse = buf.str();
    }
    //agrego al vector de cache
    cache.push_back({query, jsonResponse});

    //si es que sobrepasa cache_size, elimino el mas antiguo
    if ((int)cache.size()> CACHE_SIZE){
        cache.erase(cache.begin());
    }

    //reescribimos cache.db
    {
        ofstream out(CACHE_FILE, ios::trunc);
        for (auto& e : cache) {
            out << e.query << "|" << e.json << "\n";
        }
    }

    cout<<jsonResponse << "\n";

    return 0;
    
}