#include <iostream>
#include <vector>
#include <cstdlib> //para getenv
#include <fstream> //para archivos
#include <unistd.h> //sirve para getpid()
#include <sstream>
#include "utils.h" // Necesita trim() y limpiarConsola()
#include <limits> 

using namespace std;

// Estructura para almacenar datos de usuario en memoria
struct Usuario {
    string nombre;
    int id;
    string perfil;
    string pass;
    string username;
};

// Declaraciones de funciones locales
void cargarUsuarios(vector<Usuario>& usuarios, const string& nombreArchivo, int& contadorId);
void guardarUsuario(const Usuario& u, const string& nombreArchivo);
void crearUsuario(vector<Usuario>& usuarios, int& contadorId, const string& nombreArchivo); // Simplificado: ya no necesita usuariosAGuardar
void listarUsuarios(const vector<Usuario>& usuarios);
void eliminarUsuario(vector<Usuario>& usuarios, const string& nombreArchivo); // Simplificado

int main() {
    int opcion;
    int contadorId = 1; // Contador para asignar IDs únicos
    vector<Usuario> usuarios; // Vector para mantener los usuarios en memoria

    // Leer la variable de entorno para el archivo de usuarios
    const char* archivo_env = getenv("USER_FILE");
    string nombreArchivo = (archivo_env != nullptr) ? archivo_env : "USUARIOS.TXT"; // Usa valor por defecto si no está definida

    cout << "PID del proceso Admin: " << getpid() << endl;
    cout << "Usando archivo de usuarios: " << nombreArchivo << endl;

    // Cargar usuarios existentes del archivo al inicio
    cargarUsuarios(usuarios, nombreArchivo, contadorId);

    // Bucle principal del menú de administración
    do {
        cout << "\n====== Administrador de Usuarios ======\n";
        cout << "0. Salir\n";
        cout << "1. Crear Usuario\n";
        cout << "2. Eliminar Usuario\n";
        cout << "3. Listar Usuarios\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        // Validar entrada numérica
        if (cin.fail()) {
            cout << "Opción inválida. Ingrese un número.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpiar buffer después de leer número

        switch (opcion) {
            case 1:
                crearUsuario(usuarios, contadorId, nombreArchivo);
                break;
            case 2:
                eliminarUsuario(usuarios, nombreArchivo);
                break;
            case 3:
                listarUsuarios(usuarios);
                break;
            case 0:
                cout << "Saliendo del administrador...\n";
                break;
            default:
                cout << "Opcion incorrecta. Intente de nuevo.\n";
        }
    } while (opcion != 0);

    return 0;
}

// Guarda un nuevo usuario al final del archivo (modo append)
void guardarUsuario(const Usuario& u, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo, ios::app); // Abre en modo append
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar: " << nombreArchivo << endl;
        return;
    }
    // Escribe los 5 campos requeridos
    archivo << u.id << "," << u.nombre << "," << u.username << "," << u.perfil << "," << u.pass << "\n";
    archivo.close();
    cout << "Usuario guardado en " << nombreArchivo << endl;
}

// Carga todos los usuarios del archivo al vector en memoria
void cargarUsuarios(vector<Usuario>& usuarios, const string& nombreArchivo, int& contadorId) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Archivo " << nombreArchivo << " no encontrado. Se creará uno nuevo al guardar.\n";
        return; // No es un error crítico si el archivo no existe aún
    }

    usuarios.clear(); // Limpia el vector antes de cargar
    string linea;
    int maxId = 0; // Para rastrear el ID más alto

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string idStr, nombre, uName, perfil, pass;

        // Leer los 5 campos separados por coma
        getline(ss, idStr, ',');
        getline(ss, nombre, ',');
        getline(ss, uName, ',');
        getline(ss, perfil, ',');
        getline(ss, pass); // El resto es la contraseña

        // Validación básica de que todos los campos se leyeron
        if (idStr.empty() || nombre.empty() || uName.empty() || perfil.empty() || pass.empty()) {
            cerr << "Advertencia: Línea mal formateada ignorada -> " << linea << endl;
            continue;
        }

        Usuario u;
        try {
            u.id = stoi(idStr); // Convertir ID a entero
        } catch (const invalid_argument& e) {
            cerr << "Advertencia: ID inválido en línea -> " << linea << endl;
            continue;
        }

        u.nombre = trim(nombre);
        u.username = trim(uName);
        u.perfil = trim(perfil);
        u.pass = trim(pass); // La contraseña no necesita trim usualmente

        usuarios.push_back(u); // Añadir al vector

        // Actualizar el ID máximo encontrado para el contador
        if (u.id > maxId) {
            maxId = u.id;
        }
    }
    archivo.close();
    contadorId = maxId + 1; // El próximo ID será el siguiente al máximo encontrado
}

// Interfaz para crear un nuevo usuario
void crearUsuario(vector<Usuario>& usuarios, int& contadorId, const string& nombreArchivo){
    limpiarConsola();
    Usuario u;

    cout << "--- Crear Nuevo Usuario ---\n";
    cout << "Nombre completo del usuario: ";
    // cin.ignore() es necesario si la lectura anterior fue un número (ej. opción del menú)
    // getline(cin, u.nombre); // Si ya se limpió el buffer antes, no hace falta ignore
    getline(cin, u.nombre);


    cout << "Username (para login): ";
    getline(cin, u.username);

    // Validar que el username no esté vacío
    if (u.username.empty()) {
        cout << "Error: El username no puede estar vacío.\n";
        return;
    }

    // Validar que el username no exista ya (opcional pero recomendado)
    for(const auto& user : usuarios) {
        if (user.username == u.username) {
            cout << "Error: El username '" << u.username << "' ya existe.\n";
            return;
        }
    }

    u.id = contadorId; // Asignar el siguiente ID disponible

    // Bucle para validar el perfil
    do {
        cout << "Ingrese perfil (admin/general): ";
        cin >> u.perfil;
        // Convertir a minúsculas para comparación flexible (opcional)
        // std::transform(u.perfil.begin(), u.perfil.end(), u.perfil.begin(), ::tolower);
        if (u.perfil != "admin" && u.perfil != "general") {
            cout << "Perfil inválido. Solo se permite 'admin' o 'general'.\n";
        }
    } while (u.perfil != "admin" && u.perfil != "general");

    cout << "Ingrese la contrasena: ";
    cin >> u.pass;

    // Validar que la contraseña no esté vacía
    if (u.pass.empty()) {
        cout << "Error: La contraseña no puede estar vacía.\n";
        // Limpiar buffer por si acaso
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpiar buffer después de leer contraseña

    // Confirmación antes de guardar
    cout << "\nResumen:\n";
    cout << "ID: " << u.id << "\nNombre: " << u.nombre << "\nUsername: " << u.username << "\nPerfil: " << u.perfil << "\n";
    cout << "Desea guardar este usuario? (1: Si, Otro: No): ";
    int aux;
    cin >> aux;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpiar buffer

    if (aux == 1) {
        guardarUsuario(u, nombreArchivo); // Guarda en el archivo
        usuarios.push_back(u); // Añade al vector en memoria
        contadorId++; // Incrementa el contador para el próximo usuario
        cout << "Usuario creado y guardado.\n";
    } else {
        cout << "Creación cancelada.\n";
    }
}

// Muestra la lista de usuarios cargados en memoria
void listarUsuarios(const vector<Usuario>& usuarios) {
    limpiarConsola();
    cout << "--- Lista de Usuarios ---\n";
    if (usuarios.empty()) {
        cout << "No hay usuarios registrados.\n";
        return;
    }

    cout << "ID\tNombre Completo\tUsername\tPerfil\n";
    cout << "--------------------------------------------------\n";
    for (const Usuario& u : usuarios) {
        // Formato básico de tabla
        cout << u.id << "\t" << u.nombre << "\t\t" << u.username << "\t\t" << u.perfil << endl;
    }
}

// Elimina un usuario por ID (tanto del vector en memoria como del archivo)
void eliminarUsuario(vector<Usuario>& usuarios, const string& nombreArchivo) {
    limpiarConsola();
    int id;
    cout << "--- Eliminar Usuario ---\n";
    cout << "Ingrese el ID del usuario a eliminar: ";
    cin >> id;

    if (cin.fail()) {
        cout << "ID inválido.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    bool encontrado = false;
    for (size_t i = 0; i < usuarios.size(); ++i) {
        if (usuarios[i].id == id) {
            encontrado = true;
            cout << "Usuario encontrado:\n";
            cout << "ID: " << usuarios[i].id << "\nNombre: " << usuarios[i].nombre << "\nUsername: " << usuarios[i].username << "\nPerfil: " << usuarios[i].perfil << "\n";

            // Advertencia si es admin
            if (usuarios[i].perfil == "admin") {
                cout << "\nALERTA: Está a punto de eliminar un usuario con perfil ADMIN.\n";
            }

            cout << "Confirmar eliminación? (1: Si, Otro: No): ";
            int opcion;
            cin >> opcion;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (opcion == 1) {
                usuarios.erase(usuarios.begin() + i); // Elimina del vector en memoria

                // Reescribe el archivo completo sin el usuario eliminado
                ofstream archivo(nombreArchivo, ios::trunc); // Abre en modo truncate (borra contenido)
                if (!archivo.is_open()) {
                    cerr << "Error: No se pudo abrir el archivo para actualizar: " << nombreArchivo << endl;
                    // Opcional: ¿Restaurar el usuario en el vector si falla la escritura?
                    return;
                }
                for (const auto& uArchivo : usuarios) {
                    archivo << uArchivo.id << "," << uArchivo.nombre << ","
                            << uArchivo.username << "," << uArchivo.perfil << "," << uArchivo.pass << "\n";
                }
                archivo.close();

                cout << "Usuario con ID " << id << " eliminado correctamente.\n";
            } else {
                cout << "Eliminación cancelada.\n";
            }
            return; // Termina la función una vez encontrado (o cancelado)
        }
    }

    if (!encontrado) {
        cout << "No se encontró un usuario con el ID " << id << ".\n";
    }
}