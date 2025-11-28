#include <iostream>
#include <string>
#include <cstdlib> 
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h> 

// Librerías de Sockets (Cliente)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>

using namespace std;

// Dirección del servidor de caché (asumimos localhost)
const string SERVER_IP = "127.0.0.1"; 

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

// Función para conectar y enviar la consulta al servidor de caché
string sendQueryToCache(const string& query, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    string response = "";

    // 1. Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "\n[BUSCADOR] Error: No se pudo crear el socket del cliente.\n";
        return "";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // Convertir IP de string a binario
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "\n[BUSCADOR] Error: Dirección IP inválida o no soportada.\n";
        close(sock);
        return "";
    }

    // 2. Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "\n[BUSCADOR] Error: Fallo en la conexión al servidor de caché (" << SERVER_IP << ":" << port << ").\n";
        cerr << "Asegúrese de que el servidor 'cache.cpp' esté corriendo.\n";
        close(sock);
        return "";
    }

    // 3. Enviar la consulta (ruta_idx;palabra)
    string msg = query;
    send(sock, msg.c_str(), msg.size(), 0);
    
    // 4. Recibir la respuesta (JSON)
    char buffer[4096] = {0};
    ssize_t bytes_read = recv(sock, buffer, 4096, 0);

    if (bytes_read > 0) {
        response = string(buffer, bytes_read);
    } else if (bytes_read == 0) {
        cerr << "[BUSCADOR] Conexión cerrada por el servidor.\n";
    } else {
        perror("[BUSCADOR] Error al recibir respuesta");
    }
    
    // 5. Cerrar el socket
    close(sock);
    
    return response;
}

int main(int argc, char* argv[]) {
    // obtener y mostrar pid
    cerr << "PID del BUSCADOR SistOpe: " << getpid() << endl;
    fflush(stdout); 

    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <ruta_archivo.idx>\n";
        return 1;
    }

    // Leer el puerto de la caché
    int cachePort = 4001; // Puerto por defecto
    if (const char* p = getenv("CACHE_PORT")) {
        try { cachePort = stoi(p); } catch(...) {}
    }

    string filePath = argv[1];
    
    // obtener la palabra a buscar del usuario (ya limpia)
    string palabraBuscada = obtenerPalabra();

    if (palabraBuscada.empty()) {
        cout << "Búsqueda cancelada: palabra vacía o inválida.\n";
        return 0;
    }

    // componer la consulta para el caché: "ruta_idx;palabra"
    string consulta = filePath + ";" + palabraBuscada;
    
    cout << "\n--- Resultados de Búsqueda ---\n";
    cout << "Palabra buscada: '" << palabraBuscada << "'\n";
    cout << "Índice usado: " << filePath << "\n";
    cout << "Conectando a Caché en puerto: " << cachePort << "\n\n";

    // 4. Enviar consulta y obtener JSON de la caché
    string jsonResponse = sendQueryToCache(consulta, cachePort);

    // 5. Imprimir la respuesta JSON recibida
    if (!jsonResponse.empty()) {
        cout << jsonResponse;
        if (jsonResponse.back() != '\n') {
            cout << "\n"; 
        }
    } else {
        cout << "No se recibió respuesta del servidor de caché.\n";
    }

    cout << "-----------------------------\n";
    return 0;
}