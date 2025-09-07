#include <iostream>
#include <string>
#include <cstring>   // para strcmp
#include "usuarios.h"
#include "headers/matrices.h"
#include "juego.h"
#include "palindrome.h"
#include "funcion.h"
#include "conteo.h"

using namespace std;

// Función para mostrar el menú principal
void menu(const string &usuario, const string &perfil) {
    cout << "\n===== MENU PRINCIPAL =====\n";
    cout << "Usuario: " << usuario << " | Perfil: " << perfil << "\n";
    cout << "--------------------------\n";
    cout << "1) Admin de usuarios y perfiles\n";
    cout << "2) Multiplica matrices NxN\n";
    cout << "3) Juego\n";
    cout << "4) ¿Es palindromo?\n";
    cout << "5) Calcular f(x) = x*x + 2x + 8\n";
    cout << "6) Conteo sobre texto\n";
    cout << "0) Salir\n";
    cout << "==========================\n";
    cout << "Seleccione una opcion: ";
}

int main(int argc, char* argv[]) {
    string usuario, password, filename;

    // Procesar argumentos de ejecución
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            usuario = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            password = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[++i];
        }
    }

    // Aquí deberías validar el usuario y password contra tus perfiles
    // (por ahora dejamos un valor por defecto)
    string perfil = "GENERAL"; 
    if (usuario == "admin") {
        perfil = "ADMIN";
    }

    int opcion;
    do {
        menu(usuario, perfil);
        cin >> opcion;
        cin.ignore(); // limpiar buffer

        switch (opcion) {
            case 1:
                if (perfil == "ADMIN") {
                    adminUsuarios();  // funcion en usuarios.h
                } else {
                    cout << "⚠️ Acceso denegado. Solo ADMIN puede acceder.\n";
                }
                break;
            case 2:
                ejecutarMultiplicacion(); // funcion en matrices.h
                break;
            case 3:
                iniciarJuego(); // funcion en juego.h
                break;
            case 4:
                opcionPalindromo(); // funcion en palindrome.h
                break;
            case 5:
                calcularFuncion(); // funcion en funcion.h
                break;
            case 6:
                conteoTexto(); // funcion en conteo.h
                break;
            case 0:
                cout << "Saliendo del programa...\n";
                break;
            default:
                cout << "⚠️ Opcion no valida.\n";
        }

    } while (opcion != 0);

    return 0;
}
