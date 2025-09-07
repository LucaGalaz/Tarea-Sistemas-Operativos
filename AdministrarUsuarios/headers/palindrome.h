#ifndef PALINDROME_H
#define PALINDROME_H

#include <string>
#include <algorithm>
#include <cctype>
#include <iostream>
using namespace std;

inline bool isPalindrome(const string &text) {
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

inline void opcionPalindromo() {
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

#endif
