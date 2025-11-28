#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

LOG_FILE = os.environ.get("GAME_LOG_FILE")
OUTPUT_DIR = os.environ.get("STATS_OUTPUT_DIR")

def load():
    if not LOG_FILE:
        print("ERROR: GAME_LOG_FILE no definido")
        sys.exit(1)
    df = pd.read_csv(LOG_FILE)

    # Validar columnas correctas
    columnas_ok = {
        "turno_id", "jugador_id", "jugador_nombre",
        "equipo", "avance", "posicion_acumulada",
        "timestamp_inicio", "timestamp_fin"
    }

    if not columnas_ok.issubset(df.columns):
        print("ERROR: El CSV no tiene el formato esperado.")
        print("Columnas encontradas:", list(df.columns))
        sys.exit(1)

    # Calcular tiempo del turno
    df["tiempo_turno"] = df["timestamp_fin"] - df["timestamp_inicio"]

    print(f"OK: {len(df)} turnos cargados")
    return df


# 1) Tiempo por turno por jugador
def plot_tiempo_por_turno(df):
    plt.figure(figsize=(10,5))
    for jugador, d in df.groupby("jugador_nombre"):
        plt.plot(d["turno_id"], d["tiempo_turno"], label=jugador)

    plt.xlabel("Turno")
    plt.ylabel("Tiempo del turno (s)")
    plt.title("Tiempo por turno por jugador")
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/01_tiempo_por_turno.png")
    plt.close()


# 2) Avance acumulado por turno (por equipo)
def plot_avance_por_turno(df):
    plt.figure(figsize=(10,5))

    # Nos quedamos con el avance acumulado por turno por equipo
    for equipo, d in df.groupby("equipo"):
        plt.plot(d["turno_id"], d["posicion_acumulada"], label=equipo)

    plt.xlabel("Turno")
    plt.ylabel("Posición acumulada")
    plt.title("Progreso acumulado de cada equipo por turno")
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/02_progreso_por_equipo.png")
    plt.close()

# 3) Promedio de tirada por jugador
def plot_promedio_tirada(df):
    stats = df.groupby("jugador_nombre")["avance"].agg(["mean","min","max"])

    plt.figure(figsize=(10,5))
    stats["mean"].plot(kind="bar")
    plt.title("Promedio de tirada por jugador")
    plt.ylabel("Valor promedio del dado")
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/03_promedio_tirada.png")
    plt.close()

# 4) Ritmo de avance promedio por equipo
def plot_ritmo_avance(df):
    ritmo = df.groupby("equipo")["avance"].mean()

    plt.figure(figsize=(8,5))
    ritmo.plot(kind="bar")
    plt.title("Ritmo de avance promedio por equipo")
    plt.ylabel("Avance promedio por turno")
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/04_ritmo_avance.png")
    plt.close()

# MAIN
def main():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR, exist_ok=True)

    df = load()

    plot_tiempo_por_turno(df)
    plot_avance_por_turno(df)
    plot_promedio_tirada(df)
    plot_ritmo_avance(df)

    print("Gráficos generados en:", OUTPUT_DIR)


if __name__ == "__main__":
    main()