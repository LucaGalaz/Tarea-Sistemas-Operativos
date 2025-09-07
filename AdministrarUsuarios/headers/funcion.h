#ifndef FUNCION_H
#define FUNCION_H

#include <iostream>
using namespace std;

inline void calcularFuncion() {
    cout << "\n--- Calculo de f(x) = x*x + 2x + 8 ---\n";
    cout << "Ingrese un valor para x: ";
    double x;
    cin >> x;
    cin.ignore();
    double resultado = x*x + 2*x + 8;
    cout << "f(" << x << ") = " << resultado << "\n";
}

#endif
