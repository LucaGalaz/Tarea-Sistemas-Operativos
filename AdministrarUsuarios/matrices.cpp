#include "headers/matrices.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;

Matriz leerMatriz(const string& ruta, char sep) {
    Matriz matriz;
    ifstream file(ruta);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + ruta);
    }

    string linea;
    while (getline(file, linea)) {
        vector<double> fila;
        stringstream ss(linea);
        string valor;
        while (getline(ss, valor, sep)) {
            fila.push_back(stod(valor));
        }
        matriz.push_back(fila);
    }
    file.close();
    return matriz;
}

Matriz multiplicar(const Matriz& A, const Matriz& B) {
    int n = A.size();
    int m = B[0].size();
    int k = A[0].size();

    if (B.size() != k) {
        throw runtime_error("Dimensiones incompatibles para multiplicación");
    }

    Matriz C(n, vector<double>(m, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int x = 0; x < k; x++) {
                C[i][j] += A[i][x] * B[x][j];
            }
        }
    }
    return C;
}

void imprimirMatriz(const Matriz& M) {
    for (auto& fila : M) {
        for (auto& val : fila) {
            cout << val << " ";
        }
        cout << endl;
    }
}

void ejecutarMultiplicacion() {
    string rutaA = "matriz/A.txt";
    string rutaB = "matriz/B.txt";
    char separador = '#';

    try {
        Matriz A = leerMatriz(rutaA, separador);
        Matriz B = leerMatriz(rutaB, separador);

        cout << "Matriz A:" << endl;
        imprimirMatriz(A);

        cout << "\nMatriz B:" << endl;
        imprimirMatriz(B);

        Matriz C = multiplicar(A, B);

        cout << "\nResultado A x B:" << endl;
        imprimirMatriz(C);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}
