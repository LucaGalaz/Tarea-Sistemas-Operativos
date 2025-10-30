#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib> //getenv
#include <sys/stat.h> //stat para verificar existencia de archivo

using namespace std;

typedef vector<vector<double>> Matriz;

//funcion para verificar si un archivo existe
bool archivoExiste(const string& ruta) {
    struct stat buffer;
    return (stat(ruta.c_str(), &buffer) == 0);
}

//leer matriz desde archivo y validar NxN
Matriz leerMatriz(const string& ruta, int N) {
    ifstream file(ruta);
    if (!file.is_open()) {
        throw runtime_error("No se abrio el archivo: " + ruta);
    }

    Matriz matriz;
    string linea;
    char separador = '#';

    while (getline(file, linea)) {
        vector<double> fila;
        stringstream ss(linea);
        string valor;
        while (getline(ss, valor, separador)) {
            if (valor.empty())
                throw runtime_error("Formato no valido: elemento vacio en " + ruta);
            try {
                fila.push_back(stod(valor));
            } catch (...) {
                throw runtime_error("Valor no numerico en el archivo: " + ruta);
            }
        }
        //validar que cada fila tenga N columnas
        if (fila.size() != static_cast<size_t>(N)) // <-- CORREGIDO
            throw runtime_error("Numero de columnas no valido en " + ruta);
        matriz.push_back(fila);
    }

    //validar que el numero de filas sea N
    if (matriz.size() != static_cast<size_t>(N)) // <-- CORREGIDO
        throw runtime_error("Numero de filas no valido en " + ruta);

    return matriz;
}

//multiplicacion de matrices NxN
Matriz multiplicar(const Matriz& A, const Matriz& B) {
    int n = A.size();
    Matriz C(n, vector<double>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

//imprimir matriz
void imprimirMatriz(const Matriz& M) {
    for (auto& fila : M) {
        for (auto& val : fila) cout << val << " ";
        cout << endl;
    }
}

int main() {
    //obtener ruta de multi_matrices desde variable de entorno
    const char* multiMPath = getenv("MULTI_M");
    if (!multiMPath) {
        cerr << "Error: la variable de entorno MUTLI_M no esta definida\n";
        return 1;
    }
    cout << "Usando ruta de multiplica_mat desde MUTLI_M: " << multiMPath << endl;

    while (true) {
        cout << "\n--- Multiplicacion de matrices NxN ---\n";
        int N;
        cout << "Indique el tamano de la matriz N (3x3 ingrese 3 o 4x4 ingrese 4): ";
        cin >> N;
        if (N != 3 && N != 4) {
            cout << "Solo se permiten matrices de 3x3 o de 4x4.\n";
            continue;
        }

        string rutaA, rutaB;
        cout << "Ingrese la ruta del archivo que posee la primera matriz (Para matrices 3x3 use: A.txt y B.txt y para matrices 4x4: C.txt, D.txt) ";
        cin >> rutaA;
        cout << "Ingrese la ruta del archivo que posee la segunda matriz (Para matrices 3x3 use: A.txt y B.txt y para Matrices 4x4: C.txt, D.txt) ";
        cin >> rutaB;

        //validar existencia de archivos
        if (!archivoExiste(rutaA) || !archivoExiste(rutaB)) {
            cout << "Error: Uno o ambos archivos no existen en la ruta especificada.\n";
            continue;
        }

        char opcion;
        cout << "Desea realizar la multiplicacion de las matrices? (y/n): ";
        cin >> opcion;
        if (opcion != 'y' && opcion != 'Y') break;

        try {
            Matriz A = leerMatriz(rutaA, N);
            Matriz B = leerMatriz(rutaB, N);

            cout << "\nMatriz A:" << endl;
            imprimirMatriz(A);

            cout << "\nMatriz B:" << endl;
            imprimirMatriz(B);

            Matriz C = multiplicar(A, B);
            cout << "\nResultado A x B:" << endl;
            imprimirMatriz(C);
        } catch (const exception& e) {
            cerr << "Error al hacer la multiplicacion: " << e.what() << endl;
        }

        cout << "\nDesea realizar otra multiplicacion? (y/n): ";
        cin >> opcion;
        if (opcion != 'y' && opcion != 'Y') break;
    }

    return 0;
}