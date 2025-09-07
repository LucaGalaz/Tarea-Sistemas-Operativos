#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

typedef vector<vector<double>> Matriz;

// Leer matriz desde archivo con validación de NxN
Matriz leerMatriz(const string& ruta, char sep) {
    ifstream file(ruta);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + ruta);
    }

    Matriz matriz;
    string linea;
    int numCols = -1;

    while (getline(file, linea)) {
        vector<double> fila;
        stringstream ss(linea);
        string valor;
        while (getline(ss, valor, sep)) {
            if (valor.empty()) {
                throw runtime_error("Formato inválido: elemento vacío en " + ruta);
            }
            try {
                fila.push_back(stod(valor));
            } catch (...) {
                throw runtime_error("Valor no numérico en el archivo: " + ruta);
            }
        }
        if (numCols == -1) numCols = fila.size();
        else if (fila.size() != numCols)
            throw runtime_error("Número de columnas inconsistente en: " + ruta);

        matriz.push_back(fila);
    }

    if (matriz.empty()) throw runtime_error("Matriz vacía en: " + ruta);

    // Validar que la matriz sea cuadrada
    if (matriz.size() != matriz[0].size())
        throw runtime_error("La matriz no es cuadrada (NxN): " + ruta);

    return matriz;
}

// Multiplicación de matrices NxN
Matriz multiplicar(const Matriz& A, const Matriz& B) {
    int n = A.size();
    if (B.size() != n || B[0].size() != n)
        throw runtime_error("Las matrices no son compatibles NxN para multiplicación");

    Matriz C(n, vector<double>(n, 0));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];

    return C;
}

// Imprimir matriz
void imprimirMatriz(const Matriz& M) {
    for (auto& fila : M) {
        for (auto& val : fila) cout << val << " ";
        cout << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: " << argv[0] << " <ruta_A> <ruta_B> <separador>" << endl;
        return 1;
    }

    string rutaA = argv[1];
    string rutaB = argv[2];
    char separador = argv[3][0]; // solo primer caracter

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
        return 1;
    }

    return 0;
}
