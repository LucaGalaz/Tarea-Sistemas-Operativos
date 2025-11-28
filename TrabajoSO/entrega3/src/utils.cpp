#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <vector>
#include <set>
#include "utils.h"
#include <limits>
#include <cstdlib> // Para system()

using namespace std;

// --- Funciones de Utilidad General ---

// Limpia la consola (Multiplataforma)
void limpiarConsola() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Elimina espacios al inicio y final
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// --- Lógica de Autenticación y Permisos ---

// Verifica login contra USUARIOS.TXT (formato: id,nombre,username,perfil,pass)
string verificarLogin(const string& archivo, const string& username, const string& password) {
    ifstream file(archivo);
    if (!file.is_open()) {
        cerr << "\nError: No se pudo abrir el archivo de usuarios: " << archivo << "\n";
        return "";
    }

    string linea;
    while (getline(file, linea)) {
        stringstream ss(linea);
        string id_str, nombre_completo, uName, perf, pass;

        // Leer los 5 campos
        getline(ss, id_str, ',');
        getline(ss, nombre_completo, ',');
        getline(ss, uName, ','); // El tercer campo es el username
        getline(ss, perf, ',');
        getline(ss, pass); // El resto es la contraseña

        uName = trim(uName);
        perf = trim(perf);
        pass = trim(pass);

        // Comparar username y password
        if (uName == username && pass == password) {
            cout << "\nBienvenido " << nombre_completo << " (" << perf << ")\n";
            file.close();
            return perf; // Retorna el perfil si las credenciales son correctas
        }
    }

    file.close();
    cerr << "\nUsuario o contrasena incorrectos.\n";
    return ""; // Retorna vacío si no hay coincidencia
}

// Obtiene las opciones permitidas para un perfil desde PERFILES.TXT
set<int> obtenerOpcionesPerfil(const string& perfil, const string& archivoPerfiles) {
    set<int> opciones;
    ifstream file(archivoPerfiles);
    if (!file.is_open()) {
        cerr << "No se pudo abrir " << archivoPerfiles << "\n";
        return opciones; // Retorna set vacío si el archivo no existe
    }

    string linea;
    while (getline(file, linea)) {
        size_t pos = linea.find(';');
        if (pos == string::npos) continue; // Ignora líneas sin ';'
        string nombrePerfil = linea.substr(0, pos);
        string listaOpciones = linea.substr(pos + 1);

        nombrePerfil = trim(nombrePerfil);

        if (nombrePerfil == perfil) {
            stringstream ss(listaOpciones);
            string token;
            while (getline(ss, token, ',')) { // Separa los números por coma
                try {
                    opciones.insert(stoi(token)); // Convierte a entero y añade al set
                } catch (const exception& e) {
                    cerr << "Error al convertir token a entero: '" << token << "' - " << e.what() << endl;
                    continue;
                }
            }
            break; // Perfil encontrado, no seguir buscando
        }
    }

    file.close();
    return opciones;
}


// --- Funciones Específicas del Menú (No Modulares) ---

void conteoTexto(const string &filename) {
    if (filename.empty()) {
        cout << "No se ingreso ningun archivo con -f.\n";
        return;
    }

    // Asume que los libros están en una carpeta relativa "Libros"
    string ruta = "Libros/" + filename;
    ifstream file(ruta);
    if (!file.is_open()) {
        cout << "No se pudo abrir el archivo en: " << ruta << "\n";
        return;
    }

    int vocales = 0, consonantes = 0, especiales = 0, palabras = 0;
    bool enPalabra = false;
    string linea;

    while (getline(file, linea)) {
        for (char c : linea) {
            if (isalpha(static_cast<unsigned char>(c))) {
                char lower = tolower(c);
                if (string("aeiou").find(lower) != string::npos) { 
                    vocales++;
                } else {
                    consonantes++;
                }
                if (!enPalabra) {
                    palabras++;
                    enPalabra = true;
                }
            } else if (isspace(static_cast<unsigned char>(c))) {
                enPalabra = false;
            } else {
                especiales++;
            }
        }
        // Si la línea termina en medio de una palabra, asegúrate de contarla
    }

    file.close();

    cout << "\n--- Resumen de conteo en archivo ---\n";
    cout << "Archivo: " << ruta << "\n";
    cout << "Vocales: " << vocales << "\n";
    cout << "Consonantes: " << consonantes << "\n";
    cout << "Caracteres especiales: " << especiales << "\n";
    cout << "Palabras: " << palabras << "\n";
}

void calcularFuncion() {
    cout << "\n--- Calculo de f(x) = x*x + 2x + 8 ---\n";
    double x;

    while (true) {
        cout << "Ingrese un valor numerico (se aceptan decimales) para x: ";
        cin >> x;

        if (cin.fail()) { // Si la entrada no es un número
            cin.clear(); // Limpia el estado de error de cin
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Descarta la entrada inválida
            cout << "Entrada invalida. Intente nuevamente.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Descarta cualquier caracter extra (ej. si se ingresa "5abc")
            break; // Sale del bucle si la entrada es válida
        }
    }

    double resultado = x*x + 2*x + 8;
    cout << "f(" << x << ") = " << resultado << "\n";
}

// Placeholder para la opción Juego desde el menú (la acción real es lanzar game_client)
void iniciarJuego() {
    cout << "[Juego] Se intentará conectar al cliente del juego.\n";
    cout << "La lógica principal está en el programa 'game_client'.\n";
    cout << "Asegúrate de que el servidor 'game_server' esté corriendo.\n";
}

// Placeholder para Multiplicar Matrices (la acción real es lanzar multi_matrices)
void multiplicarMatrices() {
    cout << "[Matrices] Esta opción lanza el programa externo 'multi_matrices'.\n";
}

// Verifica si una cadena es palíndromo (ignorando no alfanuméricos y mayúsculas)
bool isPalindrome(const string &text) {
    string clean;
    for (char c : text) {
        if (isalnum(static_cast<unsigned char>(c))) { // Solo letras y números
            clean.push_back(tolower(c)); // Convertir a minúscula
        }
    }
    if (clean.empty()) return true; // Una cadena vacía o sin alfanuméricos es palíndromo
    string reversed = clean;
    reverse(reversed.begin(), reversed.end()); // Invierte la cadena limpia
    return clean == reversed; // Compara la original limpia con la invertida
}

// Interfaz para la opción Palíndromo
void opcionPalindromo() {
    cout << "\n--- Verificar Palindromo ---\n";
    cout << "Ingrese un texto: ";
    string texto;
    getline(cin, texto); // Leer línea completa, incluyendo espacios

    if (texto.empty()) {
        cout << "Texto vacío, no se puede verificar.\n";
        return;
    }

    // Ya no pide subopción, valida directamente
    if (isPalindrome(texto)) {
        cout << "'" << texto << "' ES un palindromo.\n";
    } else {
        cout << "'" << texto << "' NO es un palindromo.\n";
    }
}

// Placeholder para Admin Usuarios (la acción real es lanzar admin)
void adminUsuarios() {
    cout << "[ADMIN] Esta opción lanza el programa externo 'admin'.\n";
}