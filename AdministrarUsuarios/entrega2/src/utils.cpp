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

using namespace std;

void conteoTexto(const string &filename) {
    if (filename.empty()) {
        cout << "No se ingreso ningun archivo con -f.\n";
        return;
    }

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
                if (lower=='a'||lower=='e'||lower=='i'||lower=='o'||lower=='u') vocales++;
                else consonantes++;
                enPalabra = true;
            } else if (isspace(static_cast<unsigned char>(c))) {
                if (enPalabra) { palabras++; enPalabra = false; }
            } else {
                especiales++;
            }
        }
        if (enPalabra) { palabras++; enPalabra = false; }
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

        if (cin.fail()) {
            cin.clear(); // limpia el estado de error
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // descarta la entrada inválida
            cout << "Entrada invalida. Intente nuevamente.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break; 
        }
    }

    double resultado = x*x + 2*x + 8;
    cout << "f(" << x << ") = " << resultado << "\n";
}


void iniciarJuego() {
    cout << "[Juego] Módulo en construccion\n";
}

void multiplicarMatrices() {
    cout << "[Matrices] Multiplicacion de matrices NxN (en construccion)\n";
}

bool isPalindrome(const string &text) {
    string clean;
    for (char c : text) {
        if (isalnum(static_cast<unsigned char>(c))) clean.push_back(tolower(c));
    }
    string reversed = clean;
    reverse(reversed.begin(), reversed.end());
    return clean == reversed;
}

void opcionPalindromo() {
    cout << "\n--- Verificar Palindromo ---\n";
    cout << "Ingrese un texto: ";
    string texto;
    getline(cin, texto);

    cout << "1) Validar\n2) Cancelar\n";
    int subop;
    cin >> subop;
    cin.ignore();

    if (subop == 1) {
        if (isPalindrome(texto)) cout << "Es un palindromo.\n";
        else cout << "No es un palindromo.\n";
    } else {
        cout << "Operacion cancelada.\n";
    }
}

void adminUsuarios() {
    cout << "[ADMIN] Gestion de usuarios y perfiles (en construccion)\n";
}

// =================== Cambios aquí ===================
string verificarLogin(const string& archivo, const string& username, const string& password) {
    string perfil;

    ifstream file(archivo);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo " << archivo << "\n";
        return "";
    }

    string linea;
    while (getline(file, linea)) {
        size_t pos = 0;
        pos = linea.find(",");
        if (pos == string::npos) continue;
        linea = linea.substr(pos + 1); // ignorar ID

        pos = linea.find(",");
        if (pos == string::npos) continue;
        linea = linea.substr(pos + 1); // ignorar nombre

        pos = linea.find(",");
        if (pos == string::npos) continue;
        string uName = linea.substr(0, pos);
        linea = linea.substr(pos + 1);

        pos = linea.find(",");
        if (pos == string::npos) continue;
        string perf = linea.substr(0, pos);

        string pass = linea.substr(pos + 1);
        // eliminar posibles saltos de línea \r o \n
        pass.erase(remove(pass.begin(), pass.end(), '\r'), pass.end());
        pass.erase(remove(pass.begin(), pass.end(), '\n'), pass.end());

        if (uName == username && pass == password) {
            perfil = perf;
            cout << "\nBienvenido " << username << " (" << perfil << ")\n";
            file.close();
            return perfil;
        }
    }

    file.close();
    cerr << "\nUsuario o contraseña incorrectos.\n";
    return "";
}

//opciones según perfil desde archivo de perfiles
set<int> obtenerOpcionesPerfil(const string& perfil, const string& archivoPerfiles) {
    set<int> opciones;
    ifstream file(archivoPerfiles);
    if (!file.is_open()) {
        cerr << "No se pudo abrir " << archivoPerfiles << "\n";
        return opciones;
    }

    string linea;
    while (getline(file, linea)) {
        size_t pos = linea.find(';');
        if (pos == string::npos) continue;
        string nombrePerfil = linea.substr(0, pos);
        string listaOpciones = linea.substr(pos + 1);

        if (nombrePerfil == perfil) {
            stringstream ss(listaOpciones);
            string token;
            while (getline(ss, token, ',')) opciones.insert(stoi(token));
            break;
        }
    }

    file.close();
    return opciones;
}
