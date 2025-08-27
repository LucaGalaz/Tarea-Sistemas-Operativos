# Proyecto – Administrador de Usuarios  
**Sistemas Operativos**  
**Universidad Austral de Chile**  
**Profesor:** Dr. Luis Veas-Castillo  

---

## 1. Propósito de la aplicación
El propósito de este proyecto es desarrollar un **Administrador de Usuarios**.  
El sistema permite gestionar usuarios de manera simple a través de un menú con opciones numéricas.  

Las principales funcionalidades son:  
- **Ingresar usuarios**: se registran en memoria usando un `struct` y se guardan en un archivo externo (`USUARIOS.TXT`).  
- **Listar usuarios**: muestra todos los usuarios.  
- **Eliminar usuarios**: elimina un usuario según su `id`. Si el usuario a eliminar tiene perfil **ADMIN**, se debe mostrar un mensaje de advertencia.  
- **Salir**: opción `0`, que finaliza la ejecución.  

Cada usuario se representa mediante un `struct` con la siguiente información:  
- `id`  
- `nombre`  
- `username`  
- `password`  
- `perfil` (GENERAL o ADMIN)  

Además, se debe implementar un **struct lista de usuarios** para mantener en memoria a todos los usuarios que se van creando o cargando desde archivo.  

---

## 2. Ejecución del programa
Para compilar y ejecutar el programa se deben seguir estos pasos:  

1. Clonar el repositorio desde GitHub:  
   ```bash
   git clone https://github.com/LucaGalaz/Tarea-Sistemas-Operativos.git
   cd Tarea-Sistemas-Operativos
   
2. Compilar el código (ejemplo en C++):
   g++ main.cpp -o main

3. Ejecutar el programa:
   ./main

---

## 3. Variables de entorno
El sistema utiliza un archivo .env para definir variables de entorno.

Por ahora, solo se usará la siguiente:
**USER_FILE** → define el nombre del archivo donde se almacenan los usuarios.       
Esto permite modificar el archivo de usuarios sin necesidad de cambiar el código fuente.

---

## 4. Integrantes del grupo

   **GRUPO 7**
   - Cuevas Cárdenas, Ian Alexander.

   - Lara Briones, Marcelo Alejandro.

   - Galaz Esandi, Luca.

---

## 5. Referencias
- [W3Schools – C++ Structs](https://www.w3schools.com/cpp/cpp_structs.asp)  
- [StackOverflow – Save and load structures to file](https://stackoverflow.com/questions/74452768/c-save-and-load-structures-to-file)  
- [StackOverflow – Accessing environment variables in C](https://stackoverflow.com/questions/631664/accessing-environment-variables-in-c)  
- [GeeksforGeeks – Array of structs in C++](https://www.geeksforgeeks.org/cpp/how-to-create-an-array-of-structs-in-cpp/)
