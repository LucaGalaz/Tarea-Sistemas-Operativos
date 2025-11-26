#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# Definición de variables de entorno
# Estas variables deben estar cargadas con 'source setup' o tu .env
LOG_FILE = os.environ.get("GAME_LOG_FILE")
STATS_OUTPUT_DIR = os.environ.get("STATS_OUTPUT_DIR")

def cargar_datos():
    """
    Carga los datos del log del juego (CSV).
    Verifica que el archivo exista y no esté vacío.
    """
    if not LOG_FILE:
        print("ERROR: La variable de entorno GAME_LOG_FILE no está definida. Use 'source setup' para cargarla.")
        sys.exit(1)
    
    if not os.path.exists(LOG_FILE):
        print(f"ERROR: Archivo de log no encontrado en: {LOG_FILE}. Asegúrese de ejecutar partidas primero.")
        sys.exit(1)

    try:
        # Lee el archivo CSV
        df = pd.read_csv(LOG_FILE)
        
        # Convierte la columna timestamp a tipo datetime (opcional, pero útil)
        df['timestamp'] = pd.to_datetime(df['timestamp'], unit='s')
        
        if df.empty:
            print("ERROR: El archivo de log existe, pero está vacío.")
            sys.exit(1)
            
        print(f"Datos cargados exitosamente. {len(df)} partidas registradas.")
        return df
    except pd.errors.EmptyDataError:
        print("ERROR: El archivo de log está vacío o mal formado.")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR al cargar el log: {e}")
        sys.exit(1)


def generar_estadisticas_y_graficos(df):
    """
    Calcula y genera los 4 gráficos estadísticos del juego, guardándolos en STATS_OUTPUT_DIR.
    """
    print("Iniciando generación de 4 gráficos estadísticos...")

    # Crear directorio de salida si no existe
    if not STATS_OUTPUT_DIR:
        print("ERROR: La variable de entorno STATS_OUTPUT_DIR no está definida.")
        sys.exit(1)
    os.makedirs(STATS_OUTPUT_DIR, exist_ok=True)
    
    # -----------------------------------------------------------
    # Estadística 1: Distribución de Ganadores por Equipo
    # -----------------------------------------------------------
    plt.figure(figsize=(8, 6))
    ganadores_count = df['equipo_ganador'].value_counts()
    ganadores_count.plot(kind='bar', color='skyblue')
    plt.title('1. Distribución de Partidas Ganadas por Equipo')
    plt.xlabel('Equipo Ganador')
    plt.ylabel('Número de Victorias')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(os.path.join(STATS_OUTPUT_DIR, '01_ganadores_por_equipo.png'))
    plt.close()
    
    # -----------------------------------------------------------
    # Estadística 2: Tiempo Promedio vs. Número de Equipos
    # -----------------------------------------------------------
    plt.figure(figsize=(8, 6))
    tiempo_por_equipo = df.groupby('n_equipos')['duracion_seg'].mean()
    tiempo_por_equipo.plot(kind='bar', color='lightcoral')
    plt.title('2. Duración Promedio de Partida por Número de Equipos')
    plt.xlabel('Número de Equipos')
    plt.ylabel('Duración Promedio (segundos)')
    plt.xticks(rotation=0)
    plt.tight_layout()
    plt.savefig(os.path.join(STATS_OUTPUT_DIR, '02_tiempo_vs_equipos.png'))
    plt.close()

    # -----------------------------------------------------------
    # Estadística 3: Eficiencia por Jugador (Turnos / Jugador Total)
    # -----------------------------------------------------------
    # Calcula la métrica de eficiencia: cuántos turnos se necesitaron por jugador en total
    df['eficiencia_turnos_por_jugador'] = df['n_turnos'] / df['n_jugadores_totales']
    plt.figure(figsize=(10, 6))
    plt.scatter(df['n_jugadores_totales'], df['eficiencia_turnos_por_jugador'], color='green', alpha=0.6)
    plt.title('3. Eficiencia (Turnos/Jugador) vs. Jugadores Totales')
    plt.xlabel('Número Total de Jugadores en Partida')
    plt.ylabel('Eficiencia (Turnos por Jugador)')
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(STATS_OUTPUT_DIR, '03_eficiencia_por_jugador.png'))
    plt.close()

    # -----------------------------------------------------------
    # Estadística 4: Distribución de Duración de Partida (Histograma)
    # -----------------------------------------------------------
    plt.figure(figsize=(8, 6))
    df['duracion_seg'].hist(bins=10, color='purple', edgecolor='black', alpha=0.7)
    plt.title('4. Distribución de la Duración Total de Partidas')
    plt.xlabel('Duración (segundos)')
    plt.ylabel('Frecuencia de Partidas')
    plt.tight_layout()
    plt.savefig(os.path.join(STATS_OUTPUT_DIR, '04_distribucion_duracion.png'))
    plt.close()

    print(f"\nProceso finalizado. Los 4 gráficos fueron guardados en: {STATS_OUTPUT_DIR}")


if __name__ == "__main__":
    print("--- PROGRAMA GENERADOR DE ESTADÍSTICAS (Problema 2.B) ---")
    
    # Asegurarse de que las librerías necesarias (pandas, matplotlib) estén instaladas.
    try:
        df_juego = cargar_datos()
        generar_estadisticas_y_graficos(df_juego)
    except Exception as e:
        print(f"\nFATAL ERROR: Falló la ejecución del script. {e}")
        print("Asegúrese de haber activado su entorno virtual y haber instalado 'pip install pandas matplotlib'.")
        sys.exit(1)