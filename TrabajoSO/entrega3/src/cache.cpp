#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib> 
#include <algorithm>
#include <mutex>
#include <thread>
#include <map>

// Librerías de Sockets (POSIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <atomic>

using namespace std;

// --- ESTRUCTURAS DE CACHÉ ---
struct CacheEntry {
    string query; // Formato: "ruta_idx;palabra"
    string json;  // JSON de respuesta del motor
};

static string CACHE_FILE = "cache.db";
static mutex cache_mtx; // Mutex para proteger el acceso al archivo y la memoria caché
static vector<CacheEntry> cache;
static int CACHE_SIZE = 3; 

// --- UTILIDADES ---

CacheEntry parseLine(const string& line){
    size_t pos = line.find('|');
    if (pos == string::npos){
        return{"",""};
    }
    CacheEntry e;
    e.query = line.substr(0,pos);
    e.json = line.substr(pos+1);
    return e;
}

// Carga la caché desde el archivo
void loadCache() {
    lock_guard<mutex> lock(cache_mtx);
    ifstream in(CACHE_FILE);
    string line;
    cache.clear();
    while (getline(in, line)){
        if (line.empty()) continue;
        CacheEntry e = parseLine(line);
        if (!e.query.empty())
            cache.push_back(e);
    }
}

// Guarda la caché al archivo (reescribe todo)
void saveCache() {
    lock_guard<mutex> lock(cache_mtx);
    ofstream out(CACHE_FILE, ios::trunc);
    for (auto& e : cache) {
        out << e.query << "|" << e.json << "\n";
    }
}

// Llama al motor de búsqueda y captura la respuesta JSON
string callMotor(const string& query) {
    const char* motorPath = getenv("MOTOR_BUSQUEDA");
    if (!motorPath) {
        cerr << "ERROR: variable de entorno MOTOR_BUSQUEDA no definida.\n";
        return "{\"Respuesta\":[]}"; 
    }

    string tmpFile = "./tmp_response_" + to_string(getpid()) + ".json";
    ostringstream cmd;
    // Comando: <motor> "query" > tmp_file
    cmd << motorPath << " \"" << query << "\" > " << tmpFile;

    // system() para ejecutar el motor
    int ret = system(cmd.str().c_str());
    if (ret != 0) {
        cerr << "ERROR: fallo al ejecutar el MOTOR_DE_BUSQUEDA (código: " << ret << ").\n";
        return "{\"Respuesta\":[]}";
    }

    // Leer la respuesta JSON del archivo temporal
    string jsonResponse;
    ifstream in(tmpFile);
    if(in) {
        ostringstream buf;
        buf << in.rdbuf();
        jsonResponse = buf.str();
    } else {
        cerr << "ERROR: no se pudo leer el archivo temporal " << tmpFile << "\n";
        jsonResponse = "{\"Respuesta\":[]}"; 
    }
    
    // Limpieza: eliminar el archivo temporal
    remove(tmpFile.c_str());
    return jsonResponse;
}

// -------------------------------------------------------------------------
// LÓGICA DE PROCESAMIENTO PRINCIPAL
// -------------------------------------------------------------------------

string processQuery(const string& query) {
    string jsonResponse = "";
    bool cacheHit = false;

    // Buscar en caché
    {
        lock_guard<mutex> lock(cache_mtx);
        for (const auto& e : cache){
            if (e.query == query){
                jsonResponse = e.json;
                cacheHit = true;
                cerr << "[CACHE] Acierto de cache para: " << query << endl;
                break;
            }
        }
    }
    
    // Fallo de caché: Ir al motor
    if (!cacheHit) {
        cerr << "[CACHE] Fallo de cache. Llamando al Motor de Búsqueda...\n";
        jsonResponse = callMotor(query);

        // Actualizar caché
        if (!jsonResponse.empty()) {
            lock_guard<mutex> lock(cache_mtx);
            // Revisa si ya está en caché (otra hebra pudo haberlo añadido)
            bool alreadyIn = false;
            for (const auto& e : cache) { if (e.query == query) { alreadyIn = true; break; } }

            if (!alreadyIn) {
                 cache.push_back({query, jsonResponse});
                 if ((int)cache.size() > CACHE_SIZE){
                    cache.erase(cache.begin()); // Eliminar el más antiguo (LRU simple)
                 }
                 saveCache(); // Guardar en disco
            }
        }
    }

    return jsonResponse;
}

// -------------------------------------------------------------------------
// MANEJADOR DE CLIENTE (THREAD)
// -------------------------------------------------------------------------

void handle_client(int client_fd) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    
    // 1. Recibir la consulta (ruta_idx;palabra)
    ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0);

    if (bytes_received > 0) {
        buf[bytes_received] = '\0';
        string query(buf);
        
        // 2. Procesar la consulta
        string responseJson = processQuery(query);
        
        // 3. Enviar la respuesta JSON al cliente
        if (!responseJson.empty()) {
            responseJson += "\n"; // Añadir salto de línea para facilitar la lectura del cliente
            send(client_fd, responseJson.c_str(), responseJson.size(), 0);
        }
    } else if (bytes_received < 0) {
        perror("[CACHE SERVER] Error al recibir datos");
    }

    // 4. Cerrar la conexión
    close(client_fd);
}


// -------------------------------------------------------------------------
// MAIN DEL SERVIDOR
// -------------------------------------------------------------------------

int main() {
    cerr << "PID del Servidor de Caché: " << getpid() << endl;

    // Cargar configuración de entorno
    if (const char* p = getenv("CACHE_SIZE")) {
        try { CACHE_SIZE = max(1, stoi(p)); } catch(...) {}
    }
    int port = 4001; // Puerto por defecto para la caché
    if (const char* p = getenv("CACHE_PORT")) {
        try { port = stoi(p); } catch(...) {}
    }
    
    // Cargar caché inicial desde el archivo
    loadCache();
    cerr << "[CACHE] Caché cargada. Tamaño actual: " << cache.size() << ". Tamaño máximo: " << CACHE_SIZE << endl;

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address); 

    // 1. Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[CACHE SERVER] socket failed");
        return 1;
    }

    // Opción para reutilizar puerto inmediatamente
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("[CACHE SERVER] setsockopt failed");
        close(server_fd);
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port);       

    // 2. Bind (Asociar a puerto/dirección)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[CACHE SERVER] bind failed");
        close(server_fd);
        return 1;
    }

    // 3. Listen (Escuchar conexiones)
    if (listen(server_fd, 10) < 0) {
        perror("[CACHE SERVER] listen failed");
        close(server_fd);
        return 1;
    }

    cerr << "[CACHE SERVER] Iniciado y escuchando en el puerto " << port << "...\n";

    // 4. Bucle principal de aceptación
    while (true) {
        int client_fd;
        cerr << "[CACHE SERVER] Esperando conexión del Buscador...\n";
        
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("[CACHE SERVER] accept failed");
            continue; 
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        cerr << "[CACHE SERVER] Conexión aceptada de: " << client_ip << endl;

        // Iniciar un thread para manejar al cliente (buscador)
        thread client_thread(handle_client, client_fd);
        client_thread.detach(); 
    }

    close(server_fd);
    return 0;
}