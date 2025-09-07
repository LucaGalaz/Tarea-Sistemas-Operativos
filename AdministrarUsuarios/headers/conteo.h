#ifndef CONTEO_H
#define CONTEO_H

#include <iostream>
#include <string>
#include <cctype>
using namespace std;

inline void conteoTexto() {
    cout << "\n--- Conteo sobre texto ---\n";
    cout << "Ingrese un texto: ";
    string texto;
    getline(cin, texto);

    int vocales = 0, consonantes = 0, especiales = 0, palabras = 0;
    bool enPalabra = false;

    for (char c : texto) {
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
    if (enPalabra) palabras++; // contar última palabra

    cout << "Vocales: " << vocales << "\n";
    cout << "Consonantes: " << consonantes << "\n";
    cout << "Caracteres especiales: " << especiales << "\n";
    cout << "Palabras: " << palabras << "\n";
}

#endif
