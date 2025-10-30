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

using namespace std;

int PORT;
int GAME_BOARD_X;  
int POS_VICTORIA_C; 
int DICE_SIDES;    
int MIN_TEAMS;     
int MAX_TEAMS;     
int MIN_PLAYERS;  
int MAX_PLAYERS;  

enum GameState { WAITING, PLAYING, FINISHED };
atomic<GameState> currentState{WAITING}; // estado inicial

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

map<int, Player> players; 
map<string, Team> teams;  
mutex global_mtx; 
int nextPlayerId = 1; 
atomic<int> running{1}; 
int currentPlayerTurnId = 0; 
vector<int> playerOrder; 

vector<string> split(const string &s, char delim) {
    vector<string> out;
    string token; stringstream ss(s);
    while (getline(ss, token, delim)) if (!token.empty()) out.push_back(token);
    return out;
}

// enviar mensaje a todos
void broadcast(const string &msg) {
    lock_guard<mutex> lk(global_mtx);
    for (auto const& [id, player] : players) send(player.fd, msg.c_str(), msg.size(), 0);
}

// tirar dado
int rollDice() {
    static mt19937 gen(time(0)); 
    uniform_int_distribution<> dis(1, DICE_SIDES); 
    return dis(gen);
}

// verificar si se puede iniciar el juego
bool checkGameStartCondition() {
    if (teams.size() < static_cast<size_t>(MIN_TEAMS)) return false; 
    for (auto const& [name, team] : teams) if (team.members.size() < static_cast<size_t>(MIN_PLAYERS)) return false; 
    return true;
}

// definir orden de jugadores
void setupPlayerOrder() {
    playerOrder.clear();
    for(const auto& [id, player] : players) playerOrder.push_back(id);
    if (!playerOrder.empty()) currentPlayerTurnId = playerOrder[0]; 
    else currentPlayerTurnId = 0; 
}

// pasar turno al siguiente
void advanceTurn() {
    if (playerOrder.empty() || currentPlayerTurnId == 0) return; 
    auto it = find(playerOrder.begin(), playerOrder.end(), currentPlayerTurnId);
    if (it == playerOrder.end()) { 
        if (!playerOrder.empty()) currentPlayerTurnId = playerOrder[0];
        else { currentPlayerTurnId = 0; return; }
    } else {
        size_t currentIndex = distance(playerOrder.begin(), it);
        size_t nextIndex = (currentIndex + 1) % playerOrder.size();
        currentPlayerTurnId = playerOrder[nextIndex];
    }
    string turnMsg = "TURN;" + to_string(currentPlayerTurnId) + ";" + players[currentPlayerTurnId].name + "\n";
    broadcast(turnMsg);
}

// manejar cliente
void handle_client(int client_fd) {
    int myId;
    string myName = "anon";
    string myTeam = "";

    {
        lock_guard<mutex> lk(global_mtx);
        myId = nextPlayerId++;
        players[myId] = Player{myId, myName, client_fd, ""}; 
    }

    string welcome = "WELCOME;" + to_string(myId) + ";Bienvenido! Usa JOIN;nombre;equipo para unirte.\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);

    char buf[2048]; 

    while (running) {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (bytes_received <= 0) break;
        
        buf[bytes_received] = '\0'; 
        string msg(buf);
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();
        auto parts = split(msg, ';');
        if (parts.empty()) continue;

        string cmd = parts[0];

        if (cmd == "JOIN") { 
            if (parts.size() < 3) {
                send(client_fd, "ERROR;Formato incorrecto. Usa JOIN;nombre;equipo\n", 49, 0);
                continue;
            }
            string username = parts[1];
            string teamName = parts[2];
            string status = "OK"; 
            string startMsg = ""; 

            if (!myTeam.empty()) status = "ERROR;Ya estas en el equipo " + myTeam + "\n";
            else if (currentState != WAITING) status = "ERROR;Juego ya comenzo\n";
            else { 
                lock_guard<mutex> lk(global_mtx);
                if (teams.find(teamName) == teams.end() && teams.size() >= static_cast<size_t>(MAX_TEAMS)) status = "ERROR;Max equipos alcanzado\n";
                else if (teams.count(teamName) && teams[teamName].members.size() >= static_cast<size_t>(MAX_PLAYERS)) status = "ERROR;Equipo lleno\n";
                else {
                    myName = username;
                    myTeam = teamName;
                    players[myId].name = myName;
                    players[myId].team = myTeam;
                    if (teams.find(teamName) == teams.end()) teams[teamName] = Team{teamName, {}, 0};
                    teams[teamName].members.push_back(myId);

                    if (currentState == WAITING && checkGameStartCondition()) {
                        currentState = PLAYING; 
                        setupPlayerOrder(); 
                        startMsg = "GAME_STATE;PLAYING;El juego ha comenzado! Tablero: " + to_string(GAME_BOARD_X) +
                                   " pos. Victoria: " + to_string(POS_VICTORIA_C) + 
                                   ". Turno de: " + players[currentPlayerTurnId].name + "\n";
                    }
                }
            }

            if (status == "OK") {
                 string reply = "JOINED;" + myName + ";" + myTeam + "\n";
                 send(client_fd, reply.c_str(), reply.size(), 0);
                 string broadcast_msg = "PLAYER_JOINED;" + to_string(myId) + ";" + myName + ";" + myTeam + "\n";
                 broadcast(broadcast_msg); 
            } else send(client_fd, (status + "\n").c_str(), status.size() + 1, 0); 

            if (!startMsg.empty()) broadcast(startMsg);

        } else if (cmd == "ROLL") { 
            string errorMsg = "";
            int diceValue = 0;
            string currentTeamName = "";
            int currentTeamPos = 0;
            bool gameFinished = false;
            string winnerTeam = "";
            bool advanceTurnCalled = false;

            { 
                lock_guard<mutex> lk(global_mtx);
                if (currentState != PLAYING) errorMsg = "ERROR;Juego no comenzo o termino\n";
                else if (myId != currentPlayerTurnId) errorMsg = "ERROR;No es tu turno\n";
                else {
                    diceValue = rollDice(); 
                    currentTeamName = players[myId].team;
                    if (!currentTeamName.empty() && teams.count(currentTeamName)) {
                        teams[currentTeamName].position += diceValue; 
                        currentTeamPos = teams[currentTeamName].position;
                        if (currentTeamPos > POS_VICTORIA_C) {
                            currentState = FINISHED; 
                            gameFinished = true;
                            winnerTeam = currentTeamName;
                        } else advanceTurnCalled = true;
                    } else errorMsg = "ERROR;No estas en un equipo valido\n";
                }
            }

            if (!errorMsg.empty()) send(client_fd, errorMsg.c_str(), errorMsg.size(), 0); 
            else {
                string resultMsg = "ROLL_RESULT;" + to_string(myId) + ";" + myName + ";" + currentTeamName + ";" + to_string(diceValue) + "\n";
                broadcast(resultMsg);
                string updateMsg = "TEAM_UPDATE;" + currentTeamName + ";" + to_string(currentTeamPos) + "\n";
                broadcast(updateMsg);

                if (gameFinished) {
                    string endMsg = "GAME_OVER;WINNER;" + winnerTeam + ";Felicidades al equipo " + winnerTeam + "!\n";
                    broadcast(endMsg);
                } else if (advanceTurnCalled) advanceTurn();
            }

        } else if (cmd == "CHAT") { 
            if (currentState == FINISHED) continue; 
            string text = (msg.length() > cmd.length() + 1) ? msg.substr(cmd.length() + 1) : ""; 
            if (!text.empty()) broadcast("CHAT;" + myName + ";" + text + "\n");
        } else if (cmd == "LEAVE") break; 
        else send(client_fd, ("ERROR;Comando desconocido: '" + cmd + "'\n").c_str(), 50, 0);
    }

    close(client_fd); 

    string disconnectBroadcastMsg = ""; 
    { 
        lock_guard<mutex> lk(global_mtx);
        if (!myTeam.empty() && teams.count(myTeam)) {
            auto &vec = teams[myTeam].members;
            vec.erase(remove(vec.begin(), vec.end(), myId), vec.end());
            if (vec.empty()) teams.erase(myTeam);
        }
        players.erase(myId);
        playerOrder.erase(remove(playerOrder.begin(), playerOrder.end(), myId), playerOrder.end());

        if (currentState == PLAYING && myId == currentPlayerTurnId) {
            if (!playerOrder.empty()) {
                currentPlayerTurnId = playerOrder[0]; 
                disconnectBroadcastMsg = "TURN;" + to_string(currentPlayerTurnId) + ";" + players[currentPlayerTurnId].name + "\n";
            } else { 
                currentPlayerTurnId = 0;
                currentState = WAITING; 
                disconnectBroadcastMsg = "GAME_STATE;WAITING;Esperando nuevos jugadores...\n";
            }
        } else if (currentState == PLAYING && !checkGameStartCondition()) {
             currentState = WAITING;
             currentPlayerTurnId = 0;
             playerOrder.clear();
             disconnectBroadcastMsg = "GAME_STATE;WAITING;Condicion de inicio no cumplida\n";
        }
    }

    if (!disconnectBroadcastMsg.empty()) broadcast(disconnectBroadcastMsg);
    broadcast("PLAYER_LEFT;" + to_string(myId) + ";" + myName + "\n");
}


// leer configuracion de env
void load_env_config() {
    if (const char* env_x = getenv("GAME_BOARD_X")) GAME_BOARD_X = atoi(env_x);
    if (const char* env_c = getenv("POS_VICTORIA_C")) POS_VICTORIA_C = atoi(env_c);
    if (const char* env_r = getenv("DICE_R")) DICE_SIDES = atoi(env_r);
    if (const char* env_min_t = getenv("MIN_TEAMS")) MIN_TEAMS = atoi(env_min_t);
    if (const char* env_max_t = getenv("MAX_TEAMS")) MAX_TEAMS = atoi(env_max_t);
    if (const char* env_min_p = getenv("MIN_PLAYERS")) MIN_PLAYERS = atoi(env_min_p);
    if (const char* env_max_p = getenv("MAX_PLAYERS")) MAX_PLAYERS = atoi(env_max_p);
    if (const char* env_port = getenv("PORT")) PORT = atoi(env_port);

    if (DICE_SIDES < 1) DICE_SIDES = 1; 
    if (POS_VICTORIA_C <= 0) POS_VICTORIA_C = GAME_BOARD_X; 
    if (MIN_TEAMS < 1) MIN_TEAMS = 1;
    if (MIN_PLAYERS < 1) MIN_PLAYERS = 1;
    if (MAX_TEAMS < MIN_TEAMS) MAX_TEAMS = MIN_TEAMS; 
    if (MAX_PLAYERS < MIN_PLAYERS) MAX_PLAYERS = MIN_PLAYERS;
}

int main() {
    load_env_config(); 

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address); 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { close(server_fd); exit(EXIT_FAILURE); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);       

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { close(server_fd); exit(EXIT_FAILURE); }
    if (listen(server_fd, 10) < 0) { close(server_fd); exit(EXIT_FAILURE); }

    while (running) {
        int client_fd;
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) continue; 
        thread(handle_client, client_fd).detach(); 
    }

    close(server_fd);
    return 0;
}
