import sys
import pandas as pd
import matplotlib.pyplot as plt
import os

def main():
    if len(sys.argv) < 3:
        print("Uso: python plot_performance.py <archivo_csv> <carpeta_salida>")
        return

    csv_path = sys.argv[1]
    output_folder = sys.argv[2]

    try:
        # Leer datos
        data = pd.read_csv(csv_path)
        
        # Crear gráfico
        plt.figure(figsize=(10, 6))
        plt.plot(data['threads'], data['tiempo_ms'], marker='o', linestyle='-', color='b')
        
        plt.title('Rendimiento Indexación: Tiempo vs Threads')
        plt.xlabel('Cantidad de Threads')
        plt.ylabel('Tiempo de Ejecución (ms)')
        plt.grid(True)
        plt.xticks(data['threads']) # Asegurar que muestre los ticks de los threads usados

        # Guardar imagen 
        output_file = os.path.join(output_folder, 'grafico_rendimiento.png')
        plt.savefig(output_file)
        print(f"Gráfico guardado exitosamente en: {output_file}")
        
    except Exception as e:
        print(f"Error al generar el gráfico: {e}")

if __name__ == "__main__":
    main()