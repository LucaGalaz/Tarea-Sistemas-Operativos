#ifndef CONTEO_H
#define CONTEO_H

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
using namespace std;

inline void conteoTexto(const string &filename) {
    if (filename.empty()) {
        cout << "⚠️ No se ingreso ningun archivo con -f.\n";
        return;
    }

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "⚠️ No se pudo abrir el archivo: " << filename << "\n";
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
    cout << "Archivo: " << filename << "\n";
    cout << "Vocales: " << vocales << "\n";
    cout << "Consonantes: " << consonantes << "\n";
    cout << "Caracteres especiales: " << especiales << "\n";
    cout << "Palabras: " << palabras << "\n";
}

#endif
