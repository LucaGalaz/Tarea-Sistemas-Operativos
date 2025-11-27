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
#include <ctime>
#include <fstream>

using namespace std;

// --------------------------------------------
// VARIABLES DE CONFIGURACIÓN
// --------------------------------------------
int PORT;
int GAME_BOARD_X;
int POS_VICTORIA_C;
int DICE_SIDES;
int MIN_TEAMS;
int MAX_TEAMS;
int MIN_PLAYERS;
int MAX_PLAYERS;

enum GameState { WAITING, PLAYING, FINISHED };
atomic<GameState> currentState{WAITING};

struct Player {
    int id;
    string name;
    int fd;
    string team;
};

struct Team {
    string name;
    vector<int> members;
    int position = 0;
};

map<string, Team> teams;
map<int, Player> players;
mutex global_mtx;
int nextPlayerId = 1;
atomic<int> running{1};
int currentPlayerTurnId = 0;
vector<int> playerOrder;

time_t gameStartTime;
int partidaCounter = 0;
int turnCounter = 0;
time_t turnoStartTime = 0;

struct GameResult {
    int partida_id;
    int duracion_seg;
    int n_equipos;
    int n_jugadores_totales;
    string equipo_ganador;
    int posicion_final_ganador;
    int n_turnos;
};

// -------------------------------------------------------
// NUEVO: Ejecuta stats_generator.py automáticamente
// -------------------------------------------------------
void ejecutar_stats() {
    const char* stats_cmd = getenv("STATS_APP");
    if (!stats_cmd) {
        cerr << "WARN: STATS_APP no definida; no se generan gráficos.\n";
        return;
    }

    cout << "Ejecutando script de estadísticas...\n";

    string fullcmd = string(stats_cmd);
    int rc = system(fullcmd.c_str());

    if (rc != 0)
        cerr << "ERROR: stats_generator.py terminó con código " << rc << "\n";
    else
        cout << "✔ Gráficos generados exitosamente.\n";
}
// --------------------------------------------------------------------------------
// --- FUNCIONES AUXILIARES: SPLIT / BROADCAST / DADO ---
// --------------------------------------------------------------------------------

vector<string> split(const string &s, char delim) {
    vector<string> out;
    string token; stringstream ss(s);
    while (getline(ss, token, delim)) {
        if (!token.empty()) out.push_back(token);
    }
    return out;
}

void broadcast(const string &msg) {
    lock_guard<mutex> lk(global_mtx);
    for (auto const& [id, player] : players) {
        send(player.fd, msg.c_str(), msg.size(), 0);
    }
}

int rollDice() {
    static mt19937 gen(time(0));
    uniform_int_distribution<> dis(1, DICE_SIDES);
    return dis(gen);
}

// --------------------------------------------------------------------------------
// --- VERIFICAR CONDICIONES DE INICIO + MANEJO DE TURNOS ---
// --------------------------------------------------------------------------------

bool checkGameStartCondition() {
    if (teams.size() < (size_t)MIN_TEAMS) return false;

    for (auto const& [name, team] : teams)
        if (team.members.size() < (size_t)MIN_PLAYERS)
            return false;

    return true;
}

void setupPlayerOrder() {
    playerOrder.clear();
    for (const auto& [id, player] : players)
        playerOrder.push_back(id);

    if (!playerOrder.empty()) {
        currentPlayerTurnId = playerOrder[0];
        gameStartTime = time(nullptr);
        partidaCounter++;
        turnCounter = 0;
    }
}

void advanceTurn() {
    if (playerOrder.empty() || currentPlayerTurnId == 0) return;

    auto it = find(playerOrder.begin(), playerOrder.end(), currentPlayerTurnId);

    if (it == playerOrder.end()) {
        currentPlayerTurnId = playerOrder.empty() ? 0 : playerOrder[0];
    } else {
        size_t index = distance(playerOrder.begin(), it);
        currentPlayerTurnId = playerOrder[(index + 1) % playerOrder.size()];
    }

    turnCounter++;

    // Registrar inicio real del nuevo turno
    turnoStartTime = time(nullptr);

    string msg = "TURN;" + to_string(currentPlayerTurnId) + ";" +
                 players[currentPlayerTurnId].name + "\n";
    broadcast(msg);
}

// --------------------------------------------------------------------------------
// --- GENERAR ESTADÍSTICAS AUTOMÁTICAMENTE ---
// --------------------------------------------------------------------------------

void ejecutar_stats_generator() {
    const char* stats_app = getenv("STATS_APP");

    if (!stats_app) {
        cerr << "ADVERTENCIA: STATS_APP no está definida. No se generarán estadísticas.\n";
        return;
    }

    cout << "[STATS] Ejecutando generador: " << stats_app << endl;

    int ret = system(stats_app);
    if (ret != 0) {
        cerr << "ERROR: Falló la ejecución de stats_generator.py (code " << ret << ")\n";
    }
}

// --------------------------------------------------------------------------------
// --- MANEJADOR DE CLIENTES ---
// --------------------------------------------------------------------------------

void handle_client(int client_fd) {
    int myId;
    string myName = "anon";
    string myTeam = "";

    {   lock_guard<mutex> lk(global_mtx);
        myId = nextPlayerId++;
        players[myId] = Player{myId, myName, client_fd, ""};
    }

    string welcome = "WELCOME;" + to_string(myId) +
                     ";Bienvenido! Usa JOIN;nombre;equipo para unirte.\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);

    char buf[2048];

    // --------------------------- LOOP PRINCIPAL DEL JUGADOR --------------------------
    while (running) {
        memset(buf, 0, sizeof(buf));
        ssize_t br = recv(client_fd, buf, sizeof(buf)-1, 0);
        if (br <= 0) break;

        string msg(buf);
        msg.erase(remove(msg.begin(), msg.end(), '\n'), msg.end());
        msg.erase(remove(msg.begin(), msg.end(), '\r'), msg.end());

        auto parts = split(msg, ';');
        if (parts.empty()) continue;

        string cmd = parts[0];

        // ---------------------------- CMD JOIN ----------------------------
        if (cmd == "JOIN") {
            if (parts.size() < 3) {
                send(client_fd,
                     "ERROR;Formato JOIN incorrecto\n",
                     30, 0);
                continue;
            }

            string username = parts[1];
            string teamName = parts[2];
            string status = "OK";
            string startMsg = "";

            if (!myTeam.empty()) {
                status = "ERROR;Ya estás en un equipo.\n";
            }
            else if (currentState != WAITING) {
                status = "ERROR;El juego ya empezó.\n";
            }
            else {
                lock_guard<mutex> lk(global_mtx);

                if (!teams.count(teamName) && teams.size() >= (size_t)MAX_TEAMS)
                    status = "ERROR;Máximo de equipos alcanzado.\n";

                else if (teams.count(teamName) &&
                         teams[teamName].members.size() >= (size_t)MAX_PLAYERS)
                    status = "ERROR;Ese equipo está lleno.\n";

                else {
                    myName = username;
                    myTeam = teamName;
                    players[myId].name = myName;
                    players[myId].team = myTeam;

                    if (!teams.count(teamName))
                        teams[teamName] = Team{teamName};

                    teams[teamName].members.push_back(myId);

                    if (checkGameStartCondition() && currentState == WAITING) {
                        currentState = PLAYING;
                        setupPlayerOrder();
                        turnoStartTime = time(nullptr);
                        startMsg = "GAME_STATE;PLAYING;INICIO;Turno de:" +
                                   players[currentPlayerTurnId].name + "\n";
                    }
                }
            }

            if (status == "OK") {
                string reply = "JOINED;" + myName + ";" + myTeam + "\n";
                send(client_fd, reply.c_str(), reply.size(), 0);

                string bmsg =
                    "PLAYER_JOINED;" + to_string(myId) + ";" +
                    myName + ";" + myTeam + "\n";
                broadcast(bmsg);

                if (!startMsg.empty())
                    broadcast(startMsg);
            } else {
                send(client_fd, status.c_str(), status.size(), 0);
            }
        }

        // --------------------------------------------------------------------------------
        // --------------------------- CMD ROLL (Tirar dado) ------------------------------
        // --------------------------------------------------------------------------------
        else if (cmd == "ROLL") {

            string err = "";
            string teamName = "";
            int diceValue = 0;
            int newPos = 0;
            bool finished = false;
            string winner = "";

            long long turnoEndTimestamp = 0;

            {
                lock_guard<mutex> lk(global_mtx);

                if (currentState != PLAYING)
                    err = "ERROR;No se está jugando.\n";

                else if (myId != currentPlayerTurnId)
                    err = "ERROR;No es tu turno.\n";

                else {

                    // ===========================
                    // 1) Tiempo fin del turno
                    // ===========================
                    turnoEndTimestamp = time(nullptr);

                    // ===========================
                    // 2) Tirar dado
                    // ===========================
                    diceValue = rollDice();
                    teamName = players[myId].team;

                    // ===========================
                    // 3) Posición ANTES de sumar
                    // ===========================
                    int posAntes = teams[teamName].position;

                    // ===========================
                    // 4) Registrar en log
                    // ===========================
                    const char* log_file_path = getenv("GAME_LOG_FILE");
                    if (log_file_path) {
                        ofstream log(log_file_path, ios::app);

                        // Si el archivo está vacío → escribir encabezado
                        log.seekp(0, ios::end);
                        if (log.tellp() == 0) {
                            log << "turno_id,jugador_id,jugador_nombre,equipo,avance,posicion_acumulada,timestamp_inicio,timestamp_fin\n";
                        }

                        log << (turnCounter + 1) << ","
                            << myId << ","
                            << myName << ","
                            << teamName << ","
                            << diceValue << ","
                            << posAntes << ","
                            << turnoStartTime << ","
                            << turnoEndTimestamp << "\n";
                    }

                    // ===========================
                    // 5) Actualizar posición
                    // ===========================
                    teams[teamName].position += diceValue;
                    newPos = teams[teamName].position;

                    // ===========================
                    // 6) Verificar victoria
                    // ===========================
                    if (newPos > POS_VICTORIA_C) {
                        finished = true;
                        winner = teamName;
                        currentState = FINISHED;
                    }
                }
            } // ← ESTA llave FALTABA en tu código

            // ===========================
            // Mensajes después del lock
            // ===========================
            if (!err.empty()) {
                send(client_fd, err.c_str(), err.size(), 0);
                return;
            }

            broadcast("ROLL_RESULT;" +
                    to_string(myId) + ";" + myName + ";" +
                    teamName + ";" + to_string(diceValue) + "\n");

            broadcast("TEAM_UPDATE;" + teamName + ";" + to_string(newPos) + "\n");

            // ===========================
            // Fin de partida
            // ===========================
            if (finished) {

                ejecutar_stats_generator();  // Genera gráficos automáticamente

                broadcast("GAME_OVER;WINNER;" + winner + "\n");

                currentPlayerTurnId = 0;
                playerOrder.clear();
                return;
            }

            // ===========================
            // Avanzar turnos normales
            // ===========================
            advanceTurn();
        }


        // ----------------------------- CHAT -----------------------------
        else if (cmd == "CHAT") {
            if (parts.size() >= 2) {
                string text = msg.substr(5);
                broadcast("CHAT;" + myName + ";" + text + "\n");
            }
        }

        // ----------------------------- LEAVE ----------------------------
        else if (cmd == "LEAVE") {
            break;
        }

        else {
            send(client_fd,
                 "ERROR;Comando no válido\n",
                 25, 0);
        }
    }

    // --------------------------------------------------------------------------------
    // --- LIMPIEZA DEL JUGADOR AL DESCONECTAR ---
    // --------------------------------------------------------------------------------

    close(client_fd);

    string dcMsg = "";
    {
        lock_guard<mutex> lk(global_mtx);

        if (!myTeam.empty() && teams.count(myTeam)) {
            auto& vec = teams[myTeam].members;
            vec.erase(remove(vec.begin(), vec.end(), myId), vec.end());
            if (vec.empty())
                teams.erase(myTeam);
        }

        players.erase(myId);
        playerOrder.erase(remove(playerOrder.begin(),
                                 playerOrder.end(),
                                 myId),
                          playerOrder.end());

        if (currentState == PLAYING && myId == currentPlayerTurnId) {
            if (!playerOrder.empty()) {
                currentPlayerTurnId = playerOrder[0];
                dcMsg = "TURN;" + to_string(currentPlayerTurnId) +
                        ";" + players[currentPlayerTurnId].name + "\n";
            } else {
                currentState = WAITING;
                dcMsg = "GAME_STATE;WAITING;Jugadores insuficientes\n";
            }
        }
    }

    if (!dcMsg.empty())
        broadcast(dcMsg);

    broadcast("PLAYER_LEFT;" + to_string(myId) + ";" + myName + "\n");
}
// --------------------------------------------------------------------------------
// --- CARGA DE CONFIGURACIÓN DESDE VARIABLES DE ENTORNO ---
// --------------------------------------------------------------------------------

void load_env_config() {

    PORT = 4000;
    GAME_BOARD_X = 50;
    POS_VICTORIA_C = 45;
    DICE_SIDES = 6;
    MIN_TEAMS = 2;
    MAX_TEAMS = 4;
    MIN_PLAYERS = 1;
    MAX_PLAYERS = 3;

    cout << "Cargando configuración desde variables de entorno...\n";

    if (const char* x = getenv("GAME_BOARD_X")) GAME_BOARD_X = atoi(x);
    if (const char* c = getenv("POS_VICTORIA_C")) POS_VICTORIA_C = atoi(c);
    if (const char* r = getenv("DICE_R")) DICE_SIDES = atoi(r);
    if (const char* min_t = getenv("MIN_TEAMS")) MIN_TEAMS = atoi(min_t);
    if (const char* max_t = getenv("MAX_TEAMS")) MAX_TEAMS = atoi(max_t);
    if (const char* min_p = getenv("MIN_PLAYERS")) MIN_PLAYERS = atoi(min_p);
    if (const char* max_p = getenv("MAX_PLAYERS")) MAX_PLAYERS = atoi(max_p);
    if (const char* p = getenv("PORT")) PORT = atoi(p);

    cout << "\n--- Configuración cargada ---\n";
    cout << "PORT = " << PORT << "\n";
    cout << "TABLERO X = " << GAME_BOARD_X << "\n";
    cout << "POS_VICTORIA_C = " << POS_VICTORIA_C << "\n";
    cout << "DICE = " << DICE_SIDES << "\n";
    cout << "Equipos MIN/MAX = " << MIN_TEAMS << "/" << MAX_TEAMS << "\n";
    cout << "Jugadores MIN/MAX = " << MIN_PLAYERS << "/" << MAX_PLAYERS << "\n";
    cout << "-----------------------------\n\n";
}


// --------------------------------------------------------------------------------
// --- MAIN DEL SERVIDOR ---
// --------------------------------------------------------------------------------

int main() {
    load_env_config();

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(1);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(1);
    }

    cout << "Servidor iniciado en puerto " << PORT << endl;
    cout << "Estado: WAITING\n";
    cout << "Esperando mínimo " << MIN_TEAMS << " equipos con "
         << MIN_PLAYERS << " jugadores cada uno...\n\n";

    while (running) {
        int client_fd;

        if ((client_fd = accept(server_fd,
                                (struct sockaddr*)&address,
                                &addrlen)) < 0) {
            if (running)
                perror("accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        cout << "Nueva conexión desde " << client_ip << endl;

        thread t(handle_client, client_fd);
        t.detach();
    }

    cout << "Cerrando servidor...\n";
    close(server_fd);

    return 0;
}