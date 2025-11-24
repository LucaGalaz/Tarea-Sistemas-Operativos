#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib> // Para getenv, atoi
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <atomic> // Para std::atomic
#include <stdexcept> // Para stoi exception

using namespace std;

atomic<bool> keep_reading{true};

// Thread para leer y mostrar mensajes del servidor
void reader_thread(int sock) {
    char buf[4096]; // Buffer más grande
    while (keep_reading) {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_received = recv(sock, buf, sizeof(buf) - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                cout << "\n[!] Conexión cerrada por el servidor. Presiona Enter para salir.\n" << flush;
            } else {
                if (keep_reading) { // Solo mostrar error si no fue un cierre intencional
                    perror("recv failed");
                }
            }
            keep_reading = false; // Detener ambos bucles
            break;
        }

        buf[bytes_received] = '\0';
        
        // Imprimir directamente lo que manda el servidor
        cout << string(buf) << flush;
    }
    cout << "\n[Reader Thread] Finalizado.\n" << flush;
}

int main() {
    string server_ip = "127.0.0.1"; // IP del servidor (localhost por defecto)
    int port = 4000;              // Puerto por defecto

    // Leer el puerto de la variable de entorno PORT
    const char* envPort = getenv("PORT");
    if (envPort) {
        try {
            port = stoi(envPort); // Convertir a entero
            if (port <= 0 || port > 65535) {
                 cerr << "Advertencia: Puerto inválido (" << envPort << "). Usando puerto por defecto " << port << ".\n";
                 port = 4000; // Restaurar valor por defecto si es inválido
            }
        } catch (const invalid_argument& e) {
            cerr << "Advertencia: Valor inválido para PORT (" << envPort << "). Usando puerto por defecto " << port << ".\n";
        } catch (const out_of_range& e) {
            cerr << "Advertencia: Valor de PORT fuera de rango (" << envPort << "). Usando puerto por defecto " << port << ".\n";
        }
    } else {
        cout << "Advertencia: Variable de entorno PORT no definida. Usando puerto por defecto " << port << ".\n";
    }

    cout << "[Cliente] Intentando conectar a Servidor en " << server_ip << ":" << port << " ...\n";

    // Crear el socket del cliente
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error al crear socket");
        return 1;
    }

    // Configurar la dirección del servidor
    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // Limpiar estructura
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Puerto del servidor (convertido a network byte order)

    // Convertir la dirección IP de texto a formato binario
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Error: Dirección IP inválida o no soportada '" << server_ip << "'.\n";
        close(sock);
        return 1;
    }

    // Conectar al servidor
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(sock);
        return 1;
    }

    cout << "[Cliente] Conectado exitosamente al servidor!\n";
    cout << "-------------------------------------------------\n";
    cout << "Comandos disponibles:\n";
    cout << "  JOIN;nombre;equipo  (Ej: JOIN;Alice;EquipoAzul)\n";
    cout << "  ROLL                (Lanza el dado si es tu turno)\n";
    cout << "  CHAT;mensaje        (Ej: CHAT;Hola a todos!)\n";
    cout << "  LEAVE               (Desconecta del servidor)\n";
    cout << "-------------------------------------------------\n";
    cout << "Esperando mensaje de bienvenida del servidor...\n" << flush;

    // Iniciar el thread para leer mensajes del servidor
    thread reader(reader_thread, sock);

    // Bucle principal para leer la entrada del usuario y enviarla al servidor
    string line;
    cout << "> " << flush; // Prompt inicial
    while (keep_reading && getline(cin, line)) { // Leer línea completa
        if (!keep_reading) break; // Salir si el reader thread detectó desconexión

        if (line.empty()) {
            cout << "> " << flush; // Mostrar prompt de nuevo si la línea está vacía
            continue;
        }

        // Añadir newline al final para que el servidor lo procese correctamente
        line += "\n";
        ssize_t bytes_sent = send(sock, line.c_str(), line.size(), 0);

        if (bytes_sent < 0) {
            if (keep_reading) { // Solo mostrar error si no nos estamos desconectando
               perror("Error al enviar mensaje");
            }
            break; // Salir si falla el envío
        }

        // Si el usuario escribe "LEAVE", terminar el cliente localmente
        // El servidor también detectará el cierre del socket o el comando LEAVE
        string command_check = line.substr(0, line.find_first_of(";\n"));
        if (command_check == "LEAVE") {
            cout << "[Cliente] Solicitando desconexión...\n";
            keep_reading = false; // Indicar al reader thread que termine
            break; // Salir del bucle de envío
        }
         cout << "> " << flush; // Mostrar prompt para siguiente comando
    }

    // Señalizar al reader thread que debe terminar y cerrar el socket
    if (keep_reading) { // Si salimos por LEAVE o error de send, aseguramos cierre
        keep_reading = false;
    }
    // shutdown puede ayudar a desbloquear el recv del reader thread
    shutdown(sock, SHUT_RDWR);
    close(sock);               // Cerrar el file descriptor

    // Esperar a que el reader thread termine limpiamente
    if (reader.joinable()) {
        reader.join();
    }

    cout << "[Cliente] Programa finalizado.\n";
    return 0;
}