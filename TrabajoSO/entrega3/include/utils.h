#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <set>
#include <vector>

std::string trim(const std::string& str);
void conteoTexto(const std::string &filename);
void calcularFuncion();
void iniciarJuego(); // Usado por el menú para lanzar el cliente
void multiplicarMatrices();
bool isPalindrome(const std::string &text);
void opcionPalindromo();
void adminUsuarios();

// Firma de verificarLogin que coincide con tu USUARIOS.TXT (id,nombre,username,perfil,pass)
std::string verificarLogin(const std::string& archivo, const std::string& username, const std::string& password);
std::set<int> obtenerOpcionesPerfil(const std::string& perfil, const std::string& archivoPerfiles);
void limpiarConsola();

#endif // UTILS_H