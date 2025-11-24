#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib> // Para system()
#include <set>
#include "utils.h" // Necesita verificarLogin, obtenerOpcionesPerfil, limpiarConsola, etc.
#include <unistd.h> // Para getpid()
#include <filesystem> // Para validar carpetas
#include <limits> // Para numeric_limits

using namespace std;
namespace fs = std::filesystem;

// Variables globales para almacenar argumentos
string usuario, password, filename;

int main(int argc, char* argv[]) {
    // 1. Procesar argumentos de línea de comandos (-u, -p, -f)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) usuario = argv[++i];
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) password = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) filename = argv[++i];
    }

    // 2. Leer variables de entorno necesarias para los módulos
    const char* usuariosEnv = std::getenv("USER_FILE");
    const char* perfilesEnv = std::getenv("PERFIL_FILE");
    const char* adminSysEnv = std::getenv("ADMIN_SYS");
    const char* multiMEnv = std::getenv("MULTI_M"); // Corregido el nombre de la variable
    const char* creaIndiceEnv = std::getenv("CREATE_INDEX");
    const char* gameClientEnv = std::getenv("GAME_CLIENT"); // <-- Variable para el cliente del juego

    // Validar que todas las variables de entorno cruciales estén definidas
    // Se añade la validación para gameClientEnv
    if (!usuariosEnv || !perfilesEnv || !adminSysEnv || !multiMEnv || !creaIndiceEnv || !gameClientEnv) {
        cerr << "Error Critico: Faltan una o más variables de entorno requeridas.\n";
        cerr << "Asegúrese de ejecutar 'make setup' y exportar las variables.\n";
        return 1; // Terminar si falta configuración esencial
    }

    // Convertir a string para uso posterior
    string archivoUsuarios = usuariosEnv;
    string archivoPerfiles = perfilesEnv;
    string pathAdmin = adminSysEnv;
    string pathMulti = multiMEnv;
    string pathCreaIndice = creaIndiceEnv;
    string pathGameClient = gameClientEnv; // <-- Path del cliente

    // 3. Pedir credenciales si no se proporcionaron por argumentos
    if (usuario.empty() || password.empty()) {
        cout << "Ingrese sus credenciales:\n";
        cout << "Usuario (username): ";
        cin >> usuario;
        cout << "Contrasena: ";
        cin >> password;
        // Limpiar buffer después de leer la contraseña
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "\nPID del proceso Menu: " << getpid() << endl; // Imprime PID [cite: 61]

    // 4. Autenticar usuario y obtener perfil
    string perfilUsuario = verificarLogin(archivoUsuarios, usuario, password);
    if (perfilUsuario.empty()) {
        cerr << "Autenticación fallida. Saliendo.\n";
        return 1; // Terminar si el login falla
    }

    // 5. Cargar opciones permitidas para el perfil
    set<int> opcionesDisponibles = obtenerOpcionesPerfil(perfilUsuario, archivoPerfiles);
    if (opcionesDisponibles.empty() && perfilUsuario != "admin") { // Permitir admin sin opciones explícitas? Ojo aquí.
        cout << "Advertencia: El perfil '" << perfilUsuario << "' no tiene opciones asignadas en " << archivoPerfiles << ".\n";
    }

    int opcion;

    // 6. Bucle principal del menú
    do {
        limpiarConsola();
        cout << "\n===== MENU PRINCIPAL =====\n";
        cout << "Usuario: " << usuario << " | Perfil: " << perfilUsuario << "\n";
        cout << "--------------------------\n";

        // Mostrar opciones dinámicamente según el perfil
        // Opción 1 solo para admin (validado al cargar opciones) [cite: 63]
        if (opcionesDisponibles.count(1)) cout << "1) Admin de usuarios y perfiles\n";
        if (opcionesDisponibles.count(2)) cout << "2) Multiplica matrices NxN\n";
        if (opcionesDisponibles.count(3)) cout << "3) Juego (placeholder)\n";
        if (opcionesDisponibles.count(4)) cout << "4) ¿Es palindromo?\n";
        if (opcionesDisponibles.count(5)) cout << "5) Calcular f(x) = x*x + 2x + 8\n";
        if (opcionesDisponibles.count(6)) cout << "6) Conteo sobre texto\n";
        if (opcionesDisponibles.count(7)) cout << "7) Crea indice invertido\n";
        // --- NUEVA OPCIÓN PARA EL JUEGO ---
        if (opcionesDisponibles.count(8)) cout << "8) Crea inidce invertido paralelo\n"; // Asumiendo que 8 es la opción del juego
        // --- FIN NUEVA OPCIÓN ---
        cout << "0) Salir\n";
        cout << "Seleccione una opcion: ";

        cin >> opcion;

        // Validar entrada numérica
        if (cin.fail()) {
            cout << "Opción inválida. Ingrese un número.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1; // Asignar valor inválido
            cout << "\nPresione Enter para continuar...\n";
            cin.get();
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Validar si la opción está permitida (excepto salir)
        if (opcion != 0 && opcionesDisponibles.find(opcion) == opcionesDisponibles.end()) {
            cout << "Opción inválida o no permitida para su perfil.\n";
            cout << "\nPresione Enter para continuar...\n";
            cin.get();
            continue;
        }

        // Ejecutar la acción correspondiente
        switch (opcion) {
            case 1: { // Lanza admin [cite: 62]
                // Validación adicional (aunque ya filtrado por menú): Solo admin puede ejecutar
                if (perfilUsuario == "admin") {
                    string comando = pathAdmin; // Path desde ADMIN_SYS [cite: 64]
                    cout << "Ejecutando: " << comando << endl;
                    int result = system(comando.c_str());
                    if (result != 0) {
                        cerr << "Error al ejecutar el administrador de usuarios (código: " << result << ").\n";
                    }
                } else {
                    // Este mensaje no debería aparecer si la lógica de opcionesDisponibles es correcta
                    cout << "Acceso denegado. Solo administradores.\n";
                }
                break;
            }
            case 2: { // Lanza multi_matrices [cite: 65]
                string comando = pathMulti; // Path desde MULTI_M [cite: 66]
                cout << "Ejecutando: " << comando << endl;
                int result = system(comando.c_str());
                if (result != 0) {
                    cerr << "Error al ejecutar la multiplicacion de matrices (código: " << result << ").\n";
                }
                break;
            }
            case 3:{
                string comando = pathGameClient;  // Ejecuta game_client desde la variable de entorno
                cout << "Iniciando cliente de juego: " << comando << endl;
                int result = system(comando.c_str());
                if (result != 0) {
                    cerr << "Error al iniciar el cliente de juego.\n";
                }
                break;
            }
            case 4: // Llama a función interna
                opcionPalindromo();
                break;
            case 5: // Llama a función interna
                calcularFuncion();
                break;
            case 6: // Llama a función interna
                if (filename.empty()) {
                    cout << "Error: No se especificó un archivo con -f para esta opción.\n";
                } else {
                    conteoTexto(filename);
                }
                break;
            case 7: { // Lanza creaIndiceInvertido [cite: 78]
                string nombreArchivo, rutaCarpeta;
                cout << "\n--- Creacion de Indice Invertido ---\n";
                cout << "Ingrese el nombre del archivo de salida (debe terminar en .idx): "; // [cite: 81]
                getline(cin, nombreArchivo);

                // Validar extensión .idx [cite: 86]
                if (nombreArchivo.length() < 4 || nombreArchivo.substr(nombreArchivo.length() - 4) != ".idx") {
                    cout << "Error: El nombre del archivo debe terminar en .idx\n";
                    break;
                }

                cout << "Ingrese la ruta de la carpeta con los libros: "; // [cite: 83]
                getline(cin, rutaCarpeta);

                // Validar que la carpeta existe usando filesystem
                if (!fs::exists(rutaCarpeta) || !fs::is_directory(rutaCarpeta)) {
                    cerr << "Error: La carpeta '" << rutaCarpeta << "' no existe o no es un directorio.\n";
                    break;
                }

                // Construir comando con comillas para rutas con espacios [cite: 88]
                string comando = "\"" + pathCreaIndice + "\" \"" + nombreArchivo + "\" \"" + rutaCarpeta + "\""; // Path desde CREATE_INDEX [cite: 79]
                cout << "Ejecutando: " << comando << endl; // [cite: 84]

                int result = system(comando.c_str());
                if (result != 0) {
                    cerr << "Error al ejecutar la creacion del indice invertido (código: " << result << ").\n";
                }
                // La opción de "volver" está implícita en la pausa general del menú
                break;
            }
            // --- NUEVO CASE PARA EL JUEGO ---
             case 8: {
                const char* prog = getenv("INDICE_INVET_PARALELO");
                if (!prog){
                    cerr << "error: La variable de entorno INDICE_INVET_PARALELO no esta definida.\n";
                }
                string salidaIdx = "bin/indice.idx";
                string carpeta = "Libros";     
                string comando = string(prog) + " " + salidaIdx + " " + carpeta;
                system(comando.c_str());
                break;
            }
            // --- FIN NUEVO CASE ---
            case 0: // Salir
                cout << "Saliendo del programa...\n";
                break;
            default: // Opción numérica pero fuera de rango o no permitida
                cout << "Opción inválida.\n";
                break;
        }

        // Pausa antes de volver a mostrar el menú (excepto al salir)
        if (opcion != 0) {
            cout << "\nPresione Enter para volver al menu...\n";
            cin.get();
        }

    } while (opcion != 0);

    return 0;
}