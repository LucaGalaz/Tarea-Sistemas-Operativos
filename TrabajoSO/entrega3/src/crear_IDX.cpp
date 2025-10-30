#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "PID de crear_IDX: " << getpid() << "\n";

    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <archivo_salida.idx> <archivo_temporal>\n";
        return 1;
    }

    string archivoSalida = argv[1];
    string archivoTmp = argv[2];

    ifstream inFile(archivoTmp);
    if (!inFile) {
        cerr << "Error: no se pudo abrir el archivo temporal" << archivoTmp << "\n";
        return 1;
    }

    ofstream outFile(archivoSalida);
    if (!outFile) {
        cerr << "Error: no se pudo crear el archivo" << archivoSalida << "\n";
        return 1;
    }

    string linea;
    while (getline(inFile, linea)) {
        outFile << linea << "\n";
    }

    inFile.close();
    outFile.close();

    cout << "Archivo " << archivoSalida << " creado con exito.\n";
    return 0;
}