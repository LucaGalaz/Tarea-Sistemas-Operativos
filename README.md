# Proyecto – Menú Principal

**Sistemas Operativos**

**Universidad Austral de Chile**

**Profesor:** Dr. Luis Veas-Castillo

---

## 1. Propósito de la aplicación
El objetivo principal de este proyecto es construir un **sistema llamado MENÚ PRINCIPAL** que funcione como un administrador de usuarios. El sistema utiliza perfiles de usuario para autenticar y controlar el acceso a sus funcionalidades. El acceso al sistema se realiza mediante argumentos de ejecución y autenticación de usuarios.

### **Nuevas Funcionalidades**
El programa incorpora las siguientes funcionalidades, que se presentan en un menú de 7 opciones (incluyendo la de salir):
* **Admin de Usuarios y Perfiles**: Accesible solo para el perfil `ADMIN`. Esta funcionalidad está "en construcción".
* **Multiplicación de Matrices NxN**: Esta funcionalidad está "en construcción".
* **Juego**: Esta funcionalidad está "en construcción".
* **¿Es Palíndromo?**: Permite ingresar un texto y validar si es un palíndromo, ignorando mayúsculas y caracteres no alfanuméricos.
* **Cálculo de Función**: Permite calcular el valor de la función $f(x) = x^2 + 2x + 8$ y muestra el resultado con números reales.
* **Conteo sobre Texto**: Realiza un resumen de conteo de vocales, consonantes, caracteres especiales y palabras en un archivo de texto.
* **Salir**: La opción `0` permite salir del sistema.

---

## 2. Ejecución
El sistema está diseñado para ser compilado y ejecutado desde la consola. Se debe utilizar un `Makefile` para automatizar el proceso de compilación y un conjunto de argumentos para la ejecución y autenticación.

### **Ejecución del menú principal**
Para una ejecución completa, incluyendo limpieza de archivos y carga de variables de entorno, asegurese de estar en la carpeta del makefile y siga estos pasos en la terminal:

1.  **Limpiar archivos de compilación antiguos:**
    ```bash
    rm -rf obj/*
    rm -f bin/app2
    ```
2.  **Compilar el programa:**
    ```bash
    make all
    ```
3.  **Cargar variables de entorno desde el archivo `.env`:**
    ```bash
    export USER_FILE=data/USUARIOS.TXT
    export PERFIL_FILE=data/PERFILES.TXT
    ```
4.  **Ejecutar el programa con usuario, contraseña y archivo:**
    ```bash
    ./bin/app2 -u ivan -p 1234 -f libro_8800.txt
    ```

---

### **Ejecución del Programa de Multiplicación de Matrices**
El programa de multiplicación de matrices es un ejecutable separado. Para compilarlo y ejecutarlo debe encontrarse en la carpeta de AdministrarUsuarios y seguir estos pasos:

1.  **Compilar el programa de matrices:**
    ```bash
    g++ multi_matrices.cpp -o multi_matrices
    ```
2.  **Ejecutar el programa con las rutas de los archivos de matriz:**
    ```bash
    ./multi_matrices "matriz/A.txt" "matriz/B.txt" "#"
    ```
   Este programa recibe como argumento las rutas completas de los archivos `A.TXT` y `B.TXT` que contienen las matrices y el separador de los elementos (en este       caso, `#`)

---

## 3. Descripción de las variables de entorno
El proyecto utiliza variables de entorno para una configuración flexible sin modificar el código fuente.
* **`USER_FILE`**: Define la ruta del archivo donde se almacena la información de los usuarios registrados.
* **`PERFIL_FILE`**: Define la ruta al nuevo archivo (`PERFILES.TXT`) que contiene los perfiles de usuario y las opciones de menú a las que cada perfil tiene acceso.

---

## 4. Integrantes del grupo
**GRUPO 7**
- Cuevas Cárdenas, Ian Alexander.
- Lara Briones, Marcelo Alejandro.
- Galaz Esandi, Luca.
- Espinoza Fuenzalida, Samantha.
- Reyes Jaramillo, Felipe 
