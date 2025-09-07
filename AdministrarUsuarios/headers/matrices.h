#ifndef MATRICES_H
#define MATRICES_H

#include <vector>
#include <string>

typedef std::vector<std::vector<double>> Matriz;

// Declaración de funciones
Matriz leerMatriz(const std::string& ruta, char sep);
Matriz multiplicar(const Matriz& A, const Matriz& B);
void imprimirMatriz(const Matriz& M);
void ejecutarMultiplicacion();

#endif
