#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <stdexcept>

using namespace std;

atomic<bool> keep_reading{true}; //controla el reader thread

void reader_thread(int sock) {
    char buf[4096]; //buffer de recepcion
    while (keep_reading) {
        memset(buf,0,sizeof(buf));
        ssize_t bytes_received = recv(sock, buf, sizeof(buf)-1, 0);
        if (bytes_received <=0) {
            if (bytes_received==0) cout<<"\n[!] Conexión cerrada por el servidor.\n"<<flush;
            else if (keep_reading) perror("recv failed");
            keep_reading=false; //detener lectura si se desconecta
            break;
        }
        buf[bytes_received]='\0';
        cout<<string(buf)<<flush; //mostrar mensaje del servidor
    }
    cout<<"\n[Reader Thread] Finalizado.\n"<<flush;
}

int main() {
    string server_ip="127.0.0.1"; //ip por defecto
    int port=4000; //puerto por defecto

    const char* envPort=getenv("PORT");
    if(envPort) {
        try {
            port=stoi(envPort);
            if(port<=0 || port>65535) { cerr<<"Advertencia: puerto inválido. Usando 4000\n"; port=4000; }
        } catch(...) { cerr<<"Advertencia: PORT inválido. Usando 4000\n"; }
    }

    cout<<"[Cliente] Conectando a "<<server_ip<<":"<<port<<" ...\n";

    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){perror("socket");return 1;}

    sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);
    if(inet_pton(AF_INET,server_ip.c_str(),&serv_addr.sin_addr)<=0){cerr<<"IP inválida\n";close(sock);return 1;}

    if(connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr))<0){perror("connect");close(sock);return 1;}

    cout<<"[Cliente] Conectado exitosamente!\n";
    cout<<"-------------------------------------------------\n";
    cout<<"Comandos disponibles:\n";
    cout<<"  JOIN;nombre;equipo  (unirse a un equipo)\n";
    cout<<"  ROLL                (tirar dado si es tu turno)\n";
    cout<<"  CHAT;mensaje        (enviar mensaje)\n";
    cout<<"  LEAVE               (salir del servidor)\n";
    cout<<"-------------------------------------------------\n";
    cout<<"Esperando mensaje de bienvenida...\n"<<flush;

    thread reader(reader_thread,sock); //iniciar thread lector

    string line;
    cout<<"> "<<flush;
    while(keep_reading && getline(cin,line)){
        if(!keep_reading) break;
        if(line.empty()){cout<<"> "<<flush;continue;}
        line+="\n";
        ssize_t bytes_sent=send(sock,line.c_str(),line.size(),0);
        if(bytes_sent<0){if(keep_reading) perror("send"); break;}
        string cmd=line.substr(0,line.find_first_of(";\n"));
        if(cmd=="LEAVE"){cout<<"[Cliente] Desconectando...\n";keep_reading=false; break;}
        cout<<"> "<<flush; //prompt siguiente comando
    }

    if(keep_reading) keep_reading=false;
    shutdown(sock,SHUT_RDWR); //cerrar conexión
    close(sock);

    if(reader.joinable()) reader.join(); //esperar que termine el thread

    cout<<"[Cliente] Programa finalizado.\n";
    return 0;
}
