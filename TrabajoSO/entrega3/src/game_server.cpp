#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <ctime> // Para srand() y time() si se usa rand() simple

using namespace std;

// --- VARIABLES CONFIGURABLES DEL JUEGO (LEÍDAS DE ENV) ---
    int PORT;
    int GAME_BOARD_X;      // Tablero de X posiciones 
    int POS_VICTORIA_C;    // Condición para ganar (sobrepasar C posiciones) 
    int DICE_SIDES;         // Dado de R caras (valor máximo) 
    int MIN_TEAMS;          // Mínima cantidad de equipos para iniciar 
    int MAX_TEAMS;          // Máxima cantidad de equipos permitidos 
    int MIN_PLAYERS;        // Mínimo de jugadores por equipo para iniciar
    int MAX_PLAYERS; 

// --- ESTRUCTURAS Y ESTADOS GLOBALES ---
enum GameState { WAITING, PLAYING, FINISHED };
atomic<GameState> currentState{WAITING}; // Estado inicial: esperando jugadores

struct Player {
    int id;
    string name;
    int fd; // File descriptor del socket del cliente
    string team;
};

struct Team {
    string name;
    vector<int> members; // IDs de los jugadores en el equipo
    int position = 0; // Posición actual del equipo en el tablero
};

map<int, Player> players;         // Mapa de ID de jugador -> Datos del jugador
map<string, Team> teams;          // Mapa de Nombre de equipo -> Datos del equipo
mutex global_mtx;                 // Mutex para proteger el acceso a 'players' y 'teams'
int nextPlayerId = 1;             // Contador para asignar IDs únicos a los jugadores
atomic<int> running{1};           // Bandera para detener el servidor
int currentPlayerTurnId = 0;      // ID del jugador que tiene el turno (0 si nadie)
vector<int> playerOrder;          // Orden de los jugadores para los turnos

// --- FUNCIONES DE UTILIDAD ---

// Divide una cadena por un delimitador
vector<string> split(const string &s, char delim) {
    vector<string> out;
    string token; stringstream ss(s);
    while (getline(ss, token, delim)) {
        if (!token.empty()) out.push_back(token);
    }
    return out;
}

// Envía un mensaje a todos los clientes conectados
void broadcast(const string &msg) {
    lock_guard<mutex> lk(global_mtx);
    for (auto const& [id, player] : players) {
        send(player.fd, msg.c_str(), msg.size(), 0);
    }
}

// Simula el lanzamiento de un dado de R caras (DICE_SIDES) 
int rollDice() {
    static mt19937 gen(time(0)); // Usar time(0) como semilla
    uniform_int_distribution<> dis(1, DICE_SIDES); // Distribución [1, R] 
    return dis(gen);
}

// Verifica si se cumple la condición de inicio del juego (Premisa 4)
bool checkGameStartCondition() {
    // Necesita al menos MIN_TEAMS equipos 
    if (teams.size() < static_cast<size_t>(MIN_TEAMS)) return false; 

    // Cada equipo debe tener al menos MIN_PLAYERS jugadores 
    for (auto const& [name, team] : teams) {
        if (team.members.size() < static_cast<size_t>(MIN_PLAYERS)) return false; 
    }
    return true; // Si ambas condiciones se cumplen
}

// Establece el orden inicial de los jugadores para los turnos
void setupPlayerOrder() {
    playerOrder.clear();
    for(const auto& [id, player] : players) {
        playerOrder.push_back(id);
    }
    if (!playerOrder.empty()) {
        currentPlayerTurnId = playerOrder[0]; // El primer jugador tiene el turno
    } else {
        currentPlayerTurnId = 0; // Nadie tiene el turno si no hay jugadores
    }
}

// Avanza el turno al siguiente jugador en la lista
void advanceTurn() {
    if (playerOrder.empty() || currentPlayerTurnId == 0) return; 

    // Encontrar el índice del jugador actual en el orden
    auto it = find(playerOrder.begin(), playerOrder.end(), currentPlayerTurnId);
    if (it == playerOrder.end()) { 
        if (!playerOrder.empty()) {
             currentPlayerTurnId = playerOrder[0];
        } else {
             currentPlayerTurnId = 0; 
             return; 
        }
    } else {
        // Avanzar al siguiente índice de forma circular
        size_t currentIndex = distance(playerOrder.begin(), it);
        size_t nextIndex = (currentIndex + 1) % playerOrder.size();
        currentPlayerTurnId = playerOrder[nextIndex];
    }
    
    // Notificar quién tiene el turno (mensaje claro) 
    string turnMsg = "TURN;" + to_string(currentPlayerTurnId) + ";" + players[currentPlayerTurnId].name + "\n";
    broadcast(turnMsg);
}

// --- MANEJADOR DE CLIENTE (THREAD POR CADA JUGADOR) ---
void handle_client(int client_fd) {
    int myId;
    string myName = "anon";
    string myTeam = "";

    // Asignar ID único al nuevo jugador
    {
        lock_guard<mutex> lk(global_mtx);
        myId = nextPlayerId++;
        players[myId] = Player{myId, myName, client_fd, ""}; 
    }

    // Enviar mensaje de bienvenida con el ID asignado
    string welcome = "WELCOME;" + to_string(myId) + ";Bienvenido! Usa JOIN;nombre;equipo para unirte.\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);

    char buf[2048]; // Buffer para recibir mensajes

    // Bucle principal para recibir comandos del cliente
    while (running) {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) cout << "Jugador " << myId << " (" << myName << ") se desconectó.\n";
            else if (running) perror("recv failed"); 
            break;
        }
        
        buf[bytes_received] = '\0'; 

        string msg(buf);
        
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
            msg.pop_back();
        }
        
        auto parts = split(msg, ';');
        if (parts.empty()) continue;

        string cmd = parts[0];

        // --- Procesamiento de Comandos ---

        if (cmd == "JOIN") { 
            if (parts.size() < 3) {
                send(client_fd, "ERROR;Formato incorrecto. Usa JOIN;nombre;equipo\n", 49, 0);
                continue;
            }
            string username = parts[1];
            string teamName = parts[2];
            string status = "OK"; 
            string startMsg = ""; 

            if (!myTeam.empty()) { 
                status = "ERROR;Ya estás en el equipo " + myTeam + ". Usa LEAVE primero si quieres cambiar.\n";
            } else if (currentState != WAITING) { 
                 status = "ERROR;El juego ya ha comenzado, no puedes unirte ahora.\n";
            }
             else { 
                lock_guard<mutex> lk(global_mtx);

                if (teams.find(teamName) == teams.end() && teams.size() >= static_cast<size_t>(MAX_TEAMS)) { 
                    status = "ERROR;Se alcanzó el máximo de equipos (" + to_string(MAX_TEAMS) + "). Intenta unirte a uno existente.\n";
                }
                else if (teams.count(teamName) && teams[teamName].members.size() >= static_cast<size_t>(MAX_PLAYERS)) { 
                    status = "ERROR;El equipo '" + teamName + "' está lleno (máx " + to_string(MAX_PLAYERS) + ").\n";
                }
                else {
                    myName = username;
                    myTeam = teamName;
                    players[myId].name = myName;
                    players[myId].team = myTeam;

                    if (teams.find(teamName) == teams.end()) {
                        teams[teamName] = Team{teamName, {}, 0};
                        cout << "Equipo '" << teamName << "' creado.\n";
                    }
                    teams[teamName].members.push_back(myId);

                    cout << "Jugador " << myId << " (" << myName << ") se unió al equipo '" << myTeam << "'. Total en equipo: " << teams[myTeam].members.size() << "\n";

                    if (currentState == WAITING && checkGameStartCondition()) {
                        currentState = PLAYING; 
                        setupPlayerOrder(); 
                        startMsg = "GAME_STATE;PLAYING;El juego ha comenzado! Tablero: " + to_string(GAME_BOARD_X) +
                                          " pos. Victoria al superar: " + to_string(POS_VICTORIA_C) +
                                          ". Turno de: " + players[currentPlayerTurnId].name + "\n";
                    }
                }
            } // Fin del bloque lock_guard

            // Enviar respuesta al cliente
            if (status == "OK") {
                 string reply = "JOINED;" + myName + ";" + myTeam + "\n";
                 send(client_fd, reply.c_str(), reply.size(), 0);
                 string broadcast_msg = "PLAYER_JOINED;" + to_string(myId) + ";" + myName + ";" + myTeam + "\n";
                 broadcast(broadcast_msg); 
            } else {
                 send(client_fd, (status + "\n").c_str(), status.size() + 1, 0); 
            }

            
            if (!startMsg.empty()) {
                broadcast(startMsg);
            }

        } else if (cmd == "ROLL") { 
            string errorMsg = "";
            int diceValue = 0;
            string currentTeamName = "";
            int currentTeamPos = 0;
            bool gameFinished = false;
            string winnerTeam = "";
            bool advanceTurnCalled = false; 

            { // Bloque protegido
                lock_guard<mutex> lk(global_mtx);

                if (currentState != PLAYING) {
                    errorMsg = "ERROR;El juego no ha comenzado o ya terminó.\n";
                }
                else if (myId != currentPlayerTurnId) {
                    errorMsg = "ERROR;No es tu turno. Espera a " + (players.count(currentPlayerTurnId) ? players[currentPlayerTurnId].name : "...") + ".\n";
                }
                else {
                    diceValue = rollDice(); 
                    currentTeamName = players[myId].team;

                    if (!currentTeamName.empty() && teams.count(currentTeamName)) {
                        teams[currentTeamName].position += diceValue; 
                        currentTeamPos = teams[currentTeamName].position;

                        cout << "Jugador " << myId << " (" << myName << ") del equipo '" << currentTeamName
                             << "' sacó " << diceValue << ". Posición del equipo: " << currentTeamPos << endl;

                        if (currentTeamPos > POS_VICTORIA_C) {
                            currentState = FINISHED; 
                            gameFinished = true;
                            winnerTeam = currentTeamName;
                            cout << "¡Equipo '" << winnerTeam << "' ha ganado al superar " << POS_VICTORIA_C << "!\n";
                        } else {
                            advanceTurnCalled = true;
                        }
                    } else {
                        errorMsg = "ERROR;No perteneces a un equipo válido para tirar.\n";
                    }
                }
            } 

            // Enviar mensajes fuera del bloqueo
            if (!errorMsg.empty()) {
                send(client_fd, errorMsg.c_str(), errorMsg.size(), 0); 
            } else {
                string resultMsg = "ROLL_RESULT;" + to_string(myId) + ";" + myName + ";" + currentTeamName + ";" + to_string(diceValue) + "\n";
                broadcast(resultMsg);

                string updateMsg = "TEAM_UPDATE;" + currentTeamName + ";" + to_string(currentTeamPos) + "\n";
                broadcast(updateMsg);

                if (gameFinished) {
                    string endMsg = "GAME_OVER;WINNER;" + winnerTeam + ";Felicidades al equipo " + winnerTeam + "!\n";
                    broadcast(endMsg);
                } 
                else if (advanceTurnCalled) {
                    advanceTurn();
                }
            }

        } else if (cmd == "CHAT") { 
            if (currentState == FINISHED) continue; 
            string text = (msg.length() > cmd.length() + 1) ? msg.substr(cmd.length() + 1) : ""; 
            if (!text.empty()) {
                string chatMsg = "CHAT;" + myName + ";" + text + "\n";
                broadcast(chatMsg); 
            }
        } else if (cmd == "LEAVE") { 
            cout << "Jugador " << myId << " (" << myName << ") solicitó salir.\n";
            break; 
        } else {
            string unknownCmdMsg = "ERROR;Comando desconocido: '" + cmd + "'. Comandos: JOIN, ROLL, CHAT, LEAVE\n";
            send(client_fd, unknownCmdMsg.c_str(), unknownCmdMsg.size(), 0);
        }
    } // Fin del bucle while(running)

    // --- Limpieza al desconectar el cliente ---
    close(client_fd); 

    string disconnectBroadcastMsg = ""; 
    { 
        lock_guard<mutex> lk(global_mtx);
        cout << "Limpiando datos del jugador " << myId << " (" << myName << ").\n";
        bool wasTurnHolder = (myId == currentPlayerTurnId); 
        string teamToUpdate = myTeam; 

        if (!myTeam.empty() && teams.count(myTeam)) {
            auto &vec = teams[myTeam].members;
            vec.erase(remove(vec.begin(), vec.end(), myId), vec.end());
            cout << "Jugador " << myId << " eliminado del equipo '" << myTeam << "'. Quedan: " << vec.size() << "\n";
            if (vec.empty()) {
                cout << "Equipo '" << myTeam << "' quedó vacío y será eliminado.\n";
                teams.erase(myTeam);
            }
        }
        
        players.erase(myId);
        playerOrder.erase(remove(playerOrder.begin(), playerOrder.end(), myId), playerOrder.end());

        if (currentState == PLAYING && wasTurnHolder) {
            cout << "El jugador que tenía el turno (" << myId << ") se desconectó. Avanzando turno...\n";
            if (!playerOrder.empty()) {
                 currentPlayerTurnId = playerOrder[0]; 
                 string turnMsg = "TURN;" + to_string(currentPlayerTurnId) + ";" + players[currentPlayerTurnId].name + "\n";
                 disconnectBroadcastMsg = turnMsg;
            } else { 
                currentPlayerTurnId = 0;
                currentState = WAITING; 
                disconnectBroadcastMsg = "GAME_STATE;WAITING;Todos los jugadores se han ido. Esperando nuevos jugadores...\n";
            }
        }
         else if (currentState == PLAYING && !checkGameStartCondition()) {
             currentState = WAITING;
             currentPlayerTurnId = 0;
             playerOrder.clear();
             disconnectBroadcastMsg = "GAME_STATE;WAITING;Un jugador se fue y ya no se cumple la condición de inicio. Esperando jugadores...\n";
         }

    } // Fin del bloque lock_guard

    if (!disconnectBroadcastMsg.empty()) {
        broadcast(disconnectBroadcastMsg);
    }
 
    string leftMsg = "PLAYER_LEFT;" + to_string(myId) + ";" + myName + "\n";
    broadcast(leftMsg);

    cout << "Conexión cerrada y limpieza completa para el jugador " << myId << ".\n";
}


// Carga la configuración del juego (X, C, R, Min/Max) desde variables de entorno
void load_env_config() {
 
    cout << "Cargando configuración desde variables de entorno...\n";
    if (const char* env_x = getenv("GAME_BOARD_X")) GAME_BOARD_X = atoi(env_x); else cout << "WARN: GAME_BOARD_X no definida, usando por defecto: " << GAME_BOARD_X << endl;
    if (const char* env_c = getenv("POS_VICTORIA_C")) POS_VICTORIA_C = atoi(env_c); else cout << "WARN: POS_VICTORIA_C no definida, usando por defecto: " << POS_VICTORIA_C << endl;
    if (const char* env_r = getenv("DICE_R")) DICE_SIDES = atoi(env_r); else cout << "WARN: DICE_R no definida, usando por defecto: " << DICE_SIDES << endl;
    if (const char* env_min_t = getenv("MIN_TEAMS")) MIN_TEAMS = atoi(env_min_t); else cout << "WARN: MIN_TEAMS no definida, usando por defecto: " << MIN_TEAMS << endl;
    if (const char* env_max_t = getenv("MAX_TEAMS")) MAX_TEAMS = atoi(env_max_t); else cout << "WARN: MAX_TEAMS no definida, usando por defecto: " << MAX_TEAMS << endl;
    if (const char* env_min_p = getenv("MIN_PLAYERS")) MIN_PLAYERS = atoi(env_min_p); else cout << "WARN: MIN_PLAYERS no definida, usando por defecto: " << MIN_PLAYERS << endl;
    if (const char* env_max_p = getenv("MAX_PLAYERS")) MAX_PLAYERS = atoi(env_max_p); else cout << "WARN: MAX_PLAYERS no definida, usando por defecto: " << MAX_PLAYERS << endl;
    if (const char* env_port = getenv("PORT")) PORT = atoi(env_port); else cout << "WARN: PORT no definida, usando por defecto: " << PORT << endl;

    if (DICE_SIDES < 1) DICE_SIDES = 1; 
    if (POS_VICTORIA_C <= 0) POS_VICTORIA_C = GAME_BOARD_X; 
    if (MIN_TEAMS < 1) MIN_TEAMS = 1;
    if (MIN_PLAYERS < 1) MIN_PLAYERS = 1;
    if (MAX_TEAMS < MIN_TEAMS) MAX_TEAMS = MIN_TEAMS; 
    if (MAX_PLAYERS < MIN_PLAYERS) MAX_PLAYERS = MIN_PLAYERS;

    cout << "\n--- Configuración de Juego Cargada --- \n";
    cout << "Puerto TCP: " << PORT << "\n";
    cout << "Tamaño Tablero (X): " << GAME_BOARD_X << "\n";
    cout << "Posición Victoria (>C): " << POS_VICTORIA_C << "\n";
    cout << "Caras Dado (R): " << DICE_SIDES << "\n";
    cout << "Equipos (Min/Max): " << MIN_TEAMS << "/" << MAX_TEAMS << "\n";
    cout << "Jugadores/Equipo (Min/Max): " << MIN_PLAYERS << "/" << MAX_PLAYERS << "\n";
    cout << "-------------------------------------\n";
}

int main() {
    load_env_config(); 

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address); 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);       

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    cout << "Servidor de juego iniciado. Escuchando en el puerto " << PORT << ".\n";
    cout << "Estado actual: WAITING. Se requieren " << MIN_TEAMS << " equipos con al menos " << MIN_PLAYERS << " jugadores cada uno para iniciar.\n"; 

    while (running) {
        int client_fd;
        
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            if (running) {
                 perror("accept failed");
            }
            continue; 
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        cout << "\nNueva conexión aceptada desde " << client_ip << ". Asignando ID...\n";

        thread client_thread(handle_client, client_fd);
        client_thread.detach(); 
    }

    cout << "Cerrando el servidor...\n";
    close(server_fd);
    return 0;
}