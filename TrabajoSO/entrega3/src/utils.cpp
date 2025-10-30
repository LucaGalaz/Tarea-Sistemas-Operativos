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
#include <cstdlib>

using namespace std;

//limpia la consola
void limpiarConsola() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

//elimina espacios al inicio y final
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

//verifica login
string verificarLogin(const string& archivo, const string& username, const string& password) {
    ifstream file(archivo);
    if (!file.is_open()) {
        cerr << "error al abrir archivo: " << archivo << "\n";
        return "";
    }

    string linea;
    while (getline(file, linea)) {
        stringstream ss(linea);
        string id_str, nombre, uName, perf, pass;
        getline(ss, id_str, ',');
        getline(ss, nombre, ',');
        getline(ss, uName, ',');
        getline(ss, perf, ',');
        getline(ss, pass);

        uName = trim(uName);
        perf = trim(perf);
        pass = trim(pass);

        if (uName == username && pass == password) {
            cout << "bienvenido " << nombre << " (" << perf << ")\n";
            file.close();
            return perf;
        }
    }

    file.close();
    cerr << "usuario o contrasena incorrectos\n";
    return "";
}

//obtiene opciones para un perfil
set<int> obtenerOpcionesPerfil(const string& perfil, const string& archivoPerfiles) {
    set<int> opciones;
    ifstream file(archivoPerfiles);
    if (!file.is_open()) return opciones;

    string linea;
    while (getline(file, linea)) {
        size_t pos = linea.find(';');
        if (pos == string::npos) continue;
        string nombrePerfil = trim(linea.substr(0, pos));
        string listaOpciones = linea.substr(pos + 1);
        if (nombrePerfil == perfil) {
            stringstream ss(listaOpciones);
            string token;
            while (getline(ss, token, ',')) {
                try {
                    opciones.insert(stoi(token));
                } catch(...) { continue; }
            }
            break;
        }
    }
    file.close();
    return opciones;
}

//conteo de letras y palabras en archivo
void conteoTexto(const string &filename) {
    if (filename.empty()) {
        cout << "no se ingreso archivo\n";
        return;
    }

    string ruta = "libros/" + filename;
    ifstream file(ruta);
    if (!file.is_open()) {
        cout << "no se pudo abrir archivo: " << ruta << "\n";
        return;
    }

    int vocales=0, consonantes=0, especiales=0, palabras=0;
    bool enPalabra=false;
    string linea;
    while (getline(file, linea)) {
        for (char c : linea) {
            if (isalpha((unsigned char)c)) {
                char lower = tolower(c);
                if (string("aeiou").find(lower) != string::npos) vocales++;
                else consonantes++;
                if (!enPalabra) { palabras++; enPalabra=true; }
            } else if (isspace((unsigned char)c)) enPalabra=false;
            else especiales++;
        }
    }
    file.close();

    cout << "archivo: " << ruta << "\n";
    cout << "vocales: " << vocales << "\n";
    cout << "consonantes: " << consonantes << "\n";
    cout << "caracteres especiales: " << especiales << "\n";
    cout << "palabras: " << palabras << "\n";
}

//calculo de funcion
void calcularFuncion() {
    cout << "calculo f(x) = x*x + 2x + 8\n";
    double x;
    while (true) {
        cout << "ingrese valor numerico: ";
        cin >> x;
        if (cin.fail()) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "entrada invalida\n"; }
        else { cin.ignore(numeric_limits<streamsize>::max(), '\n'); break; }
    }
    cout << "f(" << x << ") = " << x*x + 2*x + 8 << "\n";
}

//inicio juego
void iniciarJuego() {
    cout << "se intentara conectar al cliente del juego\n";
}

//multiplicar matrices
void multiplicarMatrices() {
    cout << "lanza el programa multi_matrices\n";
}

//verifica palindromo
bool isPalindrome(const string &text) {
    string clean;
    for (char c : text) if (isalnum((unsigned char)c)) clean.push_back(tolower(c));
    if (clean.empty()) return true;
    string reversed = clean;
    reverse(reversed.begin(), reversed.end());
    return clean == reversed;
}

//opcion palindromo
void opcionPalindromo() {
    cout << "ingrese texto: ";
    string texto;
    getline(cin, texto);
    if (texto.empty()) { cout << "texto vacio\n"; return; }
    if (isPalindrome(texto)) cout << "'" << texto << "' es palindromo\n";
    else cout << "'" << texto << "' no es palindromo\n";
}
