#include <iostream>
#include <vector>
#include <cstdlib> // para el getenv
#include <fstream> // para los archivos

using namespace std;

struct Usuario {
	string nombre;
	int id;
	string perfil;
	string pass;
    string username;
};

void cargarUsuarios(vector<Usuario>& usuarios, const string& nombreArchivo, int& contadorId);
void guardarUsuario(const Usuario& u, const string& nombreArchivo);
void crearUsuario(vector<Usuario>& usuarios, vector<Usuario>& usuariosAGuardar, int& contadorId, const string& nombreArchivo);
void listarUsuarios(const vector<Usuario>& usuarios);
void eliminarUsuario(vector<Usuario>& usuarios, vector<Usuario>& usuariosAGuardar, const string& nombreArchivo);
void limpiarConsola();

int main() {
    int opcion;
    int contadorId = 1;
    vector<Usuario> usuarios;
    vector<Usuario> usuariosAGuardar;
    
    //esto es para resetear el archivo y los id
    //ofstream archivo("USUARIOS.TXT", ios::trunc); // abre el archivo y lo deja vacío
    //archivo.close();

    // Leer variable de entorno
    const char* archivo_env = getenv("USER_FILE");
    string nombreArchivo;

    if (archivo_env != nullptr) {
        nombreArchivo = archivo_env;
    } 
    else {
        nombreArchivo = "USUARIOS.TXT"; // si no se encuentra variable de entorno USER_FILE, se usa archivo por defecto
    }

    // Cargar usuarios existentes
    cargarUsuarios(usuarios, nombreArchivo, contadorId);
    
    do {
    cout << "====== Administrador de Usuarios ======\n" << endl;
    cout << "Que accion desea realizar\n 0. Salir\n 1. Crear Usuario\n 2. Eliminar Usuario\n 3. Listar Usuarios"<< endl;
    cin >> opcion;
    switch (opcion) {
            case 1:
                crearUsuario(usuarios, usuariosAGuardar, contadorId, nombreArchivo);
                break;
            case 2:
                eliminarUsuario(usuarios, usuariosAGuardar, nombreArchivo);
                //guardarUsuario(usuarios, nombreArchivo);
                break;
            case 3:
                listarUsuarios(usuarios);
                break;
            case 0:
                cout << "Saliendo del programa...\n";
                break;
            default:
                cout << "Opcion incorrecta...\n";
        }
        
    }while (opcion !=0); 
   
    return 0;
}

void guardarUsuario(const Usuario& u, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo, ios::app); // trunc = sobreescribe
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo para guardar.\n";
        return;
    }

    archivo << u.id << ","<< u.nombre << ","<< u.username << ","<< u.perfil << ","<< u.pass << "\n";

    cout << "Usuarios guardados en " << nombreArchivo << endl;
}

void cargarUsuarios(vector<Usuario>& usuarios, const string& nombreArchivo, int& contadorId) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "No se encontro el archivo " << nombreArchivo << ", se creara uno nuevo al guardar.\n";
        return;
    }

    usuarios.clear(); 

    string linea;
    while (getline(archivo, linea)) {
        Usuario u;
        size_t pos = 0;

        // id
        pos = linea.find(",");
        u.id = stoi(linea.substr(0, pos));
        linea = linea.substr(pos + 1);

        // nombre
        pos = linea.find(",");
        u.nombre = linea.substr(0, pos);
        linea = linea.substr(pos + 1);

        // username
        pos = linea.find(",");
        u.username = linea.substr(0, pos);
        linea = linea.substr(pos + 1);


        // perfil
        pos = linea.find(",");
        u.perfil = linea.substr(0, pos);

        // password
        u.pass = linea.substr(pos + 1);

        usuarios.push_back(u);

        if (u.id >= contadorId) {
            contadorId = u.id + 1; // mantener siguiente id libre
        }
    }

    archivo.close();
}

void crearUsuario(vector<Usuario>& usuarios, vector<Usuario>& usuariosAGuardar, int& contadorId, const string& nombreArchivo){
	limpiarConsola();
	Usuario u;
	int aux;
	cout << "Nombre del usuario"<< endl;
	cin.ignore();
	getline(cin, u.nombre);

        cout << "Username del usuario: " << endl;
        getline(cin, u.username);    
	
	u.id = contadorId;
	contadorId++;
	
	
	do {
		cout << "Ingrese perfil (admin/general): ";
		cin >> u.perfil;

		if (u.perfil != "admin" && u.perfil != "general") {
		    cout << "Perfil invalido. Solo se permite 'admin' o 'general'." << endl;
        }while (u.perfil != "admin" && u.perfil != "general");
	
	cout << "Ingrese su contraseña:" << endl;
	cin >> u.pass;
		
	
	cout << "Desea guardar el usuario en el archivo?" <<endl;
	cout << "1) Guardar\n2) Cancelar" << endl;
	cin >> aux; 
	if (aux == 1) {
		guardarUsuario(u, nombreArchivo);
		usuarios.push_back(u);
    		cout << "Usuario guardado en archivo.\n";
		}
	 else {
	 	
		cout << "Usuario no guardado\n";
		
	    }
}

void listarUsuarios(const vector<Usuario>& usuarios) {
    limpiarConsola();
    cout << "Lista de usuarios:" << endl;
    if (usuarios.empty()) { 
        cout << "Lista de usuarios vacía " << endl; 
        return;
    }

    // Cabecera con username incluido
    cout << "ID\tNombre\t\tUsername\tPerfil\n";

    for (const Usuario& u : usuarios) {
        cout << u.id << "\t" << u.nombre << "\t\t" << u.username << "\t\t" << u.perfil << endl;
    }
}
void eliminarUsuario(vector<Usuario>& usuarios, vector<Usuario>& usuariosAGuardar, const string& nombreArchivo) {
    limpiarConsola();
    int id;
    cout << "Ingrese el ID del usuario a eliminar: ";
    cin >> id;

    for (int i = 0; i < usuarios.size(); i++) {
        Usuario& u = usuarios[i];
        if (u.id == id) {
            //advertencia si el id es de un admin
            if (u.perfil == "Admin" || u.perfil == "ADMIN") {
                cout << "ALERTA: Está a punto de eliminar un usuario con perfil ADMIN.\n";
            }

            // confirmar si se elimina
            int opcion;
            cout << "ID de usuario a borrar: " << id << "\n";
            cout << "1) Eliminar\n";
            cout << "2) Cancelar\n";
            cin >> opcion;

            if (opcion == 1) {
                // elimina de la memoria
                usuarios.erase(usuarios.begin() + i);

                // elimina al usuario del archivo si esque estaba
                for (int j = 0; j < usuariosAGuardar.size(); j++) {
                    if (usuariosAGuardar[j].id == id) {
                        usuariosAGuardar.erase(usuariosAGuardar.begin() + j);
                        break;
                    }
                }

                // reescribe el archivo con los usuarios restantes
                ofstream archivo(nombreArchivo, ios::trunc);
                for (const auto& uArchivo : usuarios) {
                    archivo << uArchivo.id << "," << uArchivo.nombre << ","
                            << uArchivo.perfil << "," << uArchivo.pass << "\n";
                }
                archivo.close();

                cout << "Usuario con ID " << id << " eliminado de memoria y archivo.\n";
            } else {
                cout << "Usuario con ID " << id << " NO fue eliminado.\n";
            }
            return;
        }
    }
    cout << "No se encontró un usuario con ese ID.\n";
}

void limpiarConsola() {
	#ifdef _WIN32
	    system("cls");   // Para Windows
	#else
	    system("clear"); // Para Linux 
	#endif
}
