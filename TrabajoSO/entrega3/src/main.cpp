#include <iostream>
#include <vector>
#include <cstdlib>//para getenv
#include <fstream>//para archivos
#include <unistd.h>//para getpid()
#include <sstream>
#include "utils.h"//trim() y limpiarConsola()
#include <limits>

using namespace std;

struct usuario{
    string nombre;
    int id;
    string perfil;
    string pass;
    string username;
};

void cargarusuarios(vector<usuario>& usuarios,const string& archivo,int& contadorid);
void guardarusuario(const usuario& u,const string& archivo);
void crearusuario(vector<usuario>& usuarios,int& contadorid,const string& archivo);
void listarusuarios(const vector<usuario>& usuarios);
void eliminarusuario(vector<usuario>& usuarios,const string& archivo);

int main(){
    int opcion;
    int contadorid=1;
    vector<usuario> usuarios;

    const char* archivo_env=getenv("USER_FILE");
    string nombrearchivo=(archivo_env!=nullptr)?archivo_env:"USUARIOS.TXT";

    cout<<"pid del proceso admin: "<<getpid()<<endl;
    cout<<"usando archivo de usuarios: "<<nombrearchivo<<endl;

    cargarusuarios(usuarios,nombrearchivo,contadorid);//carga usuarios existentes al iniciar

    do{
        cout<<"\n====== administrador de usuarios ======\n";
        cout<<"0. salir\n1. crear usuario\n2. eliminar usuario\n3. listar usuarios\n";
        cout<<"seleccione una opcion: ";
        cin>>opcion;

        if(cin.fail()){cout<<"opcion invalida. ingrese un numero.\n"; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); continue;}
        cin.ignore(numeric_limits<streamsize>::max(),'\n');

        switch(opcion){
            case 1: crearusuario(usuarios,contadorid,nombrearchivo); break;//crear nuevo usuario
            case 2: eliminarusuario(usuarios,nombrearchivo); break;//eliminar usuario existente
            case 3: listarusuarios(usuarios); break;//mostrar todos los usuarios
            case 0: cout<<"saliendo del administrador...\n"; break;
            default: cout<<"opcion incorrecta. intente de nuevo.\n";
        }
    }while(opcion!=0);

    return 0;
}

void guardarusuario(const usuario& u,const string& archivo){
    ofstream f(archivo,ios::app);//modo append para no sobreescribir
    if(!f.is_open()){cerr<<"error: no se pudo abrir el archivo para guardar: "<<archivo<<endl; return;}
    f<<u.id<<","<<u.nombre<<","<<u.username<<","<<u.perfil<<","<<u.pass<<"\n";//guardar campos
    f.close();
    cout<<"usuario guardado en "<<archivo<<endl;
}

void cargarusuarios(vector<usuario>& usuarios,const string& archivo,int& contadorid){
    ifstream f(archivo);
    if(!f.is_open()){cout<<"archivo "<<archivo<<" no encontrado. se creara uno nuevo al guardar.\n"; return;}
    usuarios.clear();
    string linea;
    int maxid=0;
    while(getline(f,linea)){
        stringstream ss(linea);
        string idstr,nombre,uName,perfil,pass;
        getline(ss,idstr,','); getline(ss,nombre,','); getline(ss,uName,','); getline(ss,perfil,','); getline(ss,pass);
        if(idstr.empty()||nombre.empty()||uName.empty()||perfil.empty()||pass.empty()){cerr<<"advertencia: linea mal formateada ignorada -> "<<linea<<endl; continue;}
        usuario u;
        try{u.id=stoi(idstr);}catch(...){cerr<<"advertencia: id invalido -> "<<linea<<endl; continue;}
        u.nombre=trim(nombre); u.username=trim(uName); u.perfil=trim(perfil); u.pass=trim(pass);
        usuarios.push_back(u);
        if(u.id>maxid) maxid=u.id;
    }
    f.close();
    contadorid=maxid+1;//siguiente id disponible
}

void crearusuario(vector<usuario>& usuarios,int& contadorid,const string& archivo){
    limpiarConsola();
    usuario u;
    cout<<"--- crear nuevo usuario ---\n";
    cout<<"nombre completo del usuario: "; getline(cin,u.nombre);
    cout<<"username (para login): "; getline(cin,u.username);
    if(u.username.empty()){cout<<"error: el username no puede estar vacio.\n"; return;}
    for(const auto& user:usuarios) if(user.username==u.username){cout<<"error: el username '"<<u.username<<"' ya existe.\n"; return;}
    u.id=contadorid;
    do{cout<<"ingrese perfil (admin/general): "; cin>>u.perfil; if(u.perfil!="admin"&&u.perfil!="general") cout<<"perfil invalido. solo se permite 'admin' o 'general'.\n";}while(u.perfil!="admin"&&u.perfil!="general");
    cout<<"ingrese la contrasena: "; cin>>u.pass; if(u.pass.empty()){cout<<"error: la contrasena no puede estar vacia.\n"; cin.ignore(numeric_limits<streamsize>::max(),'\n'); return;}
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cout<<"\nresumen:\nid: "<<u.id<<"\nnombre: "<<u.nombre<<"\nusername: "<<u.username<<"\nperfil: "<<u.perfil<<"\ndesea guardar este usuario? (1: si, otro: no): ";
    int aux; cin>>aux; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    if(aux==1){guardarusuario(u,archivo); usuarios.push_back(u); contadorid++; cout<<"usuario creado y guardado.\n";}
    else cout<<"creacion cancelada.\n";
}

void listarusuarios(const vector<usuario>& usuarios){
    limpiarConsola();
    cout<<"--- lista de usuarios ---\n";
    if(usuarios.empty()){cout<<"no hay usuarios registrados.\n"; return;}
    cout<<"id\tnombre completo\tusername\tperfil\n--------------------------------------------------\n";
    for(const usuario& u:usuarios) cout<<u.id<<"\t"<<u.nombre<<"\t\t"<<u.username<<"\t\t"<<u.perfil<<endl;//muestra tabla
}

void eliminarusuario(vector<usuario>& usuarios,const string& archivo){
    limpiarConsola();
    int id;
    cout<<"--- eliminar usuario ---\ningrese el id del usuario a eliminar: "; cin>>id;
    if(cin.fail()){cout<<"id invalido.\n"; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return;}
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    bool encontrado=false;
    for(size_t i=0;i<usuarios.size();++i){
        if(usuarios[i].id==id){
            encontrado=true;
            cout<<"usuario encontrado:\nid: "<<usuarios[i].id<<"\nnombre: "<<usuarios[i].nombre<<"\nusername: "<<usuarios[i].username<<"\nperfil: "<<usuarios[i].perfil<<"\n";
            if(usuarios[i].perfil=="admin") cout<<"\nalerta: esta a punto de eliminar un usuario con perfil admin.\n";//advertencia
            cout<<"confirmar eliminacion? (1: si, otro: no): "; int opcion; cin>>opcion; cin.ignore(numeric_limits<streamsize>::max(),'\n');
            if(opcion==1){
                usuarios.erase(usuarios.begin()+i);//elimina del vector
                ofstream f(archivo,ios::trunc);//reescribe archivo sin usuario eliminado
                if(!f.is_open()){cerr<<"error: no se pudo abrir el archivo para actualizar: "<<archivo<<endl; return;}
                for(const auto& uArchivo:usuarios) f<<uArchivo.id<<","<<uArchivo.nombre<<","<<uArchivo.username<<","<<uArchivo.perfil<<","<<uArchivo.pass<<"\n";
                f.close();
                cout<<"usuario con id "<<id<<" eliminado correctamente.\n";
            }else cout<<"eliminacion cancelada.\n";
            return;
        }
    }
    if(!encontrado) cout<<"no se encontro un usuario con el id "<<id<<".\n";
}
