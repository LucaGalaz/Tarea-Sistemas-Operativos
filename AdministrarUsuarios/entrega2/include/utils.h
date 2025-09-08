#include <iostream>
#include <string>
#include <set>
using namespace std;

void conteoTexto(const string &filename);
void calcularFuncion();
void iniciarJuego();
void multiplicarMatrices();
bool isPalindrome(const string &text);
void opcionPalindromo();
void adminUsuarios();
string verificarLogin(const std::string& archivo, const std::string& username, const std::string& password);

// Nueva función para obtener opciones según perfil
set<int> obtenerOpcionesPerfil(const string& perfil, const string& archivoPerfiles);
