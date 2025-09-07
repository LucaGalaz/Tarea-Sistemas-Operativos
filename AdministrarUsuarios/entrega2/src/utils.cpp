#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include "utils.h"

using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
using namespace std;

void conteoTexto(const string &filename) {
    if (filename.empty()) {
        cout << "⚠️ No se ingreso ningun archivo con -f.\n";
        return;
    }

    // 🔹 Ruta fija a la carpeta "Libros"
    string ruta = "Libros/" + filename;

    ifstream file(ruta);
    if (!file.is_open()) {
        cout << "⚠️ No se pudo abrir el archivo en: " << ruta << "\n";
        return;
    }

    int vocales = 0, consonantes = 0, especiales = 0, palabras = 0;
    bool enPalabra = false;
    string linea;

    while (getline(file, linea)) {
        for (char c : linea) {
            if (isalpha(static_cast<unsigned char>(c))) {
                char lower = tolower(c);
                if (lower=='a'||lower=='e'||lower=='i'||lower=='o'||lower=='u')
                    vocales++;
                else
                    consonantes++;
                enPalabra = true;
            } else if (isspace(static_cast<unsigned char>(c))) {
                if (enPalabra) {
                    palabras++;
                    enPalabra = false;
                }
            } else {
                especiales++;
            }
        }
        if (enPalabra) { // fin de palabra al terminar la línea
            palabras++;
            enPalabra = false;
        }
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
    cout << "Ingrese un valor para x: ";
    double x;
    cin >> x;
    cin.ignore();
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
        if (isalnum(static_cast<unsigned char>(c))) {
            clean.push_back(tolower(c));
        }
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
        if (isPalindrome(texto))
            cout << "✅ Es un palindromo.\n";
        else
            cout << "❌ No es un palindromo.\n";
    } else {
        cout << "Operacion cancelada.\n";
    }
}

void adminUsuarios() {
    cout << "[ADMIN] Gestion de usuarios y perfiles (en construccion)\n";
}