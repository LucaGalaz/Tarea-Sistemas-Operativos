# Proyecto Sistemas Operativos - INFO198

- Universidad Austral de Chile
- Integrantes: Ian Cuevas, Marcelo Lara, Luca Galaz, Samantha Espinoza, Felipe Reyes
- Profesor: Dr. Luis Veas-Castillo

## Descripción
Sistema modular en C++ y Python que integra administración de usuarios, procesamiento de texto con índices invertidos y juego multiplayer. Desarrollado en cuatro entregas incrementales.

## Requisitos
- g++ con C++17
- Linux/Unix
- pthread
- Python 3.x (pandas, matplotlib)
- Make

## Estructura
```
bin/ # Ejecutables
logs/ # Registros
stats/ # Gráficos
src/ # Código fuente
Libros/  # Archivos texto
Makefile
.env # Variables de entorno
env.sh
```

## Compilación
```bash
make clean # Limpiar
make all # Compilar todo
source env.sh # Exporta variables de entorno
./bin/menu -u lvc -p 1111 -f libro_8800.txt # Ingreso al menú
./bin/game_server # Abre servidor del juego
```

---

# ENTREGA 1: Administrador de Usuarios

## Variables de Entorno
```bash
export USER_FILE=USUARIOS.TXT
```

## Funcionalidades
- Crear, eliminar y listar usuarios
- Struct: id, nombre, username, password, perfil
- Perfiles: ADMIN y GENERAL

## Formato USUARIOS.TXT
```
1,Juan Perez,jperez,admin,1234
```

---

# ENTREGA 2: Menú Principal y Funcionalidades Básicas

## Variables de Entorno
```bash
export USER_FILE=USUARIOS.TXT
export PERFIL_FILE=PERFILES.TXT
export ADMIN_SYS=bin/admin.exe
export MULTI_M=bin/multi_matrices.exe
```

## Funcionalidades

### Autenticación
```bash
./bin/menu -u lvc -p 1111 -f libro_8800.txt
```

### Gestión de Perfiles
Archivo PERFILES.TXT controla permisos por perfil:
```
admin;0,1,2,3,4,5,6
general;0,2,3,4,5,6
```

### Multiplicación de Matrices
- Soporta 3x3 y 4x4
- Separador: #
- Validación de formato

### Funciones del Menú
- Palíndromo (ignora espacios/mayúsculas)
- f(x) = x² + 2x + 8
- Conteo: vocales, consonantes, especiales, palabras

---

# ENTREGA 3: Índice Invertido Paralelo y Juego

## Variables de Entorno
```bash
# Índice Invertido
export CREATE_INDEX=bin/creaIndiceInvertido.exe
export INDICE_INVET_PARALELO=bin/creaIndiceInvertidoParalelo
export N_THREADS=5
export N_LOTE=4

# Juego
export GAME_CLIENT=bin/game_client.exe
export PORT=4000
export GAME_BOARD_X=50
export POS_VICTORIA_C=50
export DICE_R=30
export MIN_TEAMS=2
export MAX_TEAMS=4
export MIN_PLAYERS=2
export MAX_PLAYERS=5
export GAME_LOG_FILE=logs/game_log.csv
```
## Funcionalidades

### Índice Invertido Paralelo

### Características
- N threads configurables
- Procesamiento por lotes
- Genera MAPA-LIBROS con IDs numéricos

### Juego Multiplayer

### Mecánica
- Cliente-servidor con sockets
- Tablero de X posiciones
- Equipos y jugadores configurables
- Dado de R caras
- Condición victoria: superar C posiciones

### Comandos
```
JOIN;nombre;equipo  - Unirse
ROLL               - Tirar dado
CHAT;mensaje       - Enviar mensaje
LEAVE              - Salir
```

### Ejecución
```bash
./bin/game_server # Terminal 1
./bin/game_client # Terminal 2+ ó por menú
```

### Log
```
turno_id,jugador_id,jugador_nombre,equipo,avance,posicion_acumulada,timestamp_inicio,timestamp_fin
```

---

# ENTREGA 4: Buscador y Análisis

## Variables de Entorno
```bash
# Buscador
export BUSCADOR_SistOpe=bin/buscador_SistOpe
export CACHE=bin/cache
export MOTOR_BUSQUEDA=bin/motor_busqueda
export TOPK=3
export CACHE_SIZE=10

# Análisis
export BENCHMARK_APP=bin/benchmark
export PLOT_SCRIPT=src/plot_performance.py
export STATS_FOLDER=stats
export STATS_APP="python3 src/stats_generator.py"
export STATS_OUTPUT_DIR=stats
```
## Funcionalidades

### Sistema de Búsqueda

### 1. Buscador
- Valida archivo .idx
- Limpia consultas
- Muestra PID

### 2. Caché
- Almacena consultas recientes
- Persistencia en cache.db
- Tamaño: CACHE_SIZE

### 3. Motor
- Algoritmo TOPK
- Mapeo ID → nombre libro
- Respuesta JSON: `{"Respuesta":[{"Libro":"nombre","score":N}]}`

### Análisis de Rendimiento

### Benchmark Threads
- Array: [1,2,3,4,5,6,8,12,16]
- Mide tiempo por configuración
- Genera gráfico automático
```bash
./bin/benchmark
# Salida: stats/grafico.png
```

### Estadísticas Juego
Genera 4 gráficos automáticamente:
1. Tiempo por turno por jugador
2. Progreso acumulado por equipo
3. Promedio de tirada por jugador
4. Ritmo de avance por equipo
```bash
python3 src/stats_generator.py
# Salida: stats/game_plots/
```

---

# Notas Importantes

- Servidor juego antes que clientes
- Cargar variables antes de ejecutar
