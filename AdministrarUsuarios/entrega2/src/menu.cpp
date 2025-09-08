#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>  // para getenv
#include <set>
#include "utils.h"

using namespace std;

//para compilar: make y luego ./bin/app2

int main(int argc, char* argv[]) {
    string usuario, password, filename;

    //procesar argumentos de ejecución
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) usuario = argv[++i];
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) password = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) filename = argv[++i];
    }
    
    //leer variables de entorno
    const char* usuariosEnv = std::getenv("USER_FILE");
    const char* perfilesEnv = std::getenv("PERFIL_FILE");

    if (!usuariosEnv || !perfilesEnv) {
        cerr << "Error: no se encontraron las variables de entorno USER_FILE o PERFIL_FILE.\n";
        return 1;
    }

    string archivoUsuarios = usuariosEnv;
    string archivoPerfiles = perfilesEnv;

    // validar login
    string perfil = verificarLogin(archivoUsuarios, usuario, password);
    if (perfil.empty()) return 1;

    // obtener opciones permitidas
    set<int> opcionesPermitidas = obtenerOpcionesPerfil(perfil, archivoPerfiles);

    int opcion;
    do {
        cout << "\n===== MENU PRINCIPAL =====\n";
        cout << "Usuario: " << usuario << " | Perfil: " << perfil << "\n";
        cout << "--------------------------\n";

        if (opcionesPermitidas.count(1)) cout << "1) Admin de usuarios y perfiles\n";
        if (opcionesPermitidas.count(2)) cout << "2) Multiplica matrices NxN\n";
        if (opcionesPermitidas.count(3)) cout << "3) Juego\n";
        if (opcionesPermitidas.count(4)) cout << "4) ¿Es palindromo?\n";
        if (opcionesPermitidas.count(5)) cout << "5) Calcular f(x) = x*x + 2x + 8\n";
        if (opcionesPermitidas.count(6)) cout << "6) Conteo sobre texto\n";
        cout << "0) Salir\n";
        cout << "==========================\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();

        if (!opcionesPermitidas.count(opcion) && opcion != 0) {
            cout << "Opcion no permitida para tu perfil.\n";
            continue;
        }

        switch (opcion) {
            case 1: adminUsuarios(); break;
            case 2: multiplicarMatrices(); break;
            case 3: iniciarJuego(); break;
            case 4: opcionPalindromo(); break;
            case 5: calcularFuncion(); break;
            case 6: conteoTexto(filename); break;
            case 0: cout << "Saliendo del programa...\n"; break;
        }

    } while (opcion != 0);

    return 0;
}
