import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from pathlib import Path
import warnings
warnings.filterwarnings('ignore')

# Configuración de estilo
plt.style.use('seaborn-v0_8')
sns.set_palette("husl")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

def crear_directorio_graficas():
    """Crea el directorio para guardar las gráficas"""
    Path("./output/graficas").mkdir(parents=True, exist_ok=True)

def renombrar_algoritmo(alg):
    """Renombra algoritmos para mejor presentación"""
    if alg.startswith("Mejor-P2("):
        return "Mejor Práctica 2"
    return alg

def procesar_datos():
    """Procesa los datos siguiendo la misma lógica del script original"""
    # Carga el CSV
    df = pd.read_csv("./output/results.csv")
    
    # Limpieza de nombres de instancia
    df["Instancia"] = df["Instancia"].apply(lambda x: x.split("/")[-1])
    
    # Conservamos el nombre original para BL
    df["AlgOriginal"] = df["Algoritmo"]
    df["Algoritmo"] = df["Algoritmo"].apply(renombrar_algoritmo)
    
    # Construir la fila BL
    bl_rows = []
    for inst, grupo in df.groupby("Instancia"):
        ls = grupo[grupo["AlgOriginal"].isin(["LocalSearch-LSall", "LocalSearch-BLsmall"])]
        if not ls.empty:
            mejor = ls.loc[ls["Fitness"].idxmax()].copy()
            mejor["Algoritmo"] = "BL"
            bl_rows.append(mejor)
    
    df_bl = pd.DataFrame(bl_rows)
    df = df[~df["AlgOriginal"].isin(["LocalSearch-LSall", "LocalSearch-BLsmall"])]
    df = pd.concat([df, df_bl], ignore_index=True)
    
    # Filtrar algoritmos deseados
    nombre_tabla = {
        "RandomSearch": "Random",
        "Greedy": "Greedy",
        "BL": "BL",
        "Mejor Práctica 2": "Mejor Práctica 2",
        "BMB": "BMB",
        "ILS": "ILS",
        "ES": "ES",
        "ILS-ES": "ILS-ES",
        "GRASP-NOBL": "GRASP-NOBL",
        "GRASP-SIBL": "GRASP-SIBL"
    }
    
    df = df[df["Algoritmo"].isin(nombre_tabla.keys())].copy()
    df["AlgoritmoTabla"] = df["Algoritmo"].map(nombre_tabla)
    
    # Recalcular posiciones
    df["Position"] = (
        df
        .groupby("Instancia")["Fitness"]
        .rank(method="first", ascending=False)
        .astype(int)
    )
    
    return df

def grafica_fitness_por_algoritmo(df):
    """Gráfica de fitness promedio por algoritmo"""
    plt.figure(figsize=(14, 8))
    
    # Calcular promedios
    fitness_promedio = df.groupby("AlgoritmoTabla")["Fitness"].mean().sort_values(ascending=False)
    
    # Crear gráfica de barras
    bars = plt.bar(range(len(fitness_promedio)), fitness_promedio.values, 
                   color=sns.color_palette("viridis", len(fitness_promedio)))
    
    plt.xlabel("Algoritmos")
    plt.ylabel("Fitness Promedio")
    plt.title("Comparación de Fitness Promedio por Algoritmo", fontsize=16, fontweight='bold')
    plt.xticks(range(len(fitness_promedio)), fitness_promedio.index, rotation=45, ha='right')
    
    # Añadir valores en las barras
    for i, bar in enumerate(bars):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                f'{height:.2f}', ha='center', va='bottom', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig("./output/graficas/fitness_promedio_algoritmos.png", dpi=300, bbox_inches='tight')
    plt.show()

def grafica_posicion_promedio(df):
    """Gráfica de posición promedio (ranking) - mejor posición arriba"""
    plt.figure(figsize=(14, 8))
    
    # Calcular posiciones promedio y ordenar de mejor (menor posición) a peor
    pos_promedio = df.groupby("AlgoritmoTabla")["Position"].mean().sort_values()
    
    # Crear gráfica de barras horizontales - invertir para que mejor esté arriba
    colors = sns.color_palette("RdYlGn", len(pos_promedio))
    bars = plt.barh(range(len(pos_promedio)), pos_promedio.values[::-1], color=colors[::-1])
    
    plt.xlabel("Posición Promedio (menor es mejor)")
    plt.ylabel("Algoritmos")
    plt.title("Ranking Promedio de Algoritmos (Mejor → Peor)", fontsize=16, fontweight='bold')
    plt.yticks(range(len(pos_promedio)), pos_promedio.index[::-1])
    
    # Añadir valores en las barras
    for i, bar in enumerate(bars):
        width = bar.get_width()
        plt.text(width + 0.1, bar.get_y() + bar.get_height()/2.,
                f'{width:.2f}', ha='left', va='center', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig("./output/graficas/ranking_promedio_algoritmos.png", dpi=300, bbox_inches='tight')
    plt.show()



def grafica_boxplot_fitness(df):
    """Boxplot de fitness por algoritmo"""
    plt.figure(figsize=(14, 8))
    
    # Ordenar algoritmos por mediana de fitness
    orden = df.groupby("AlgoritmoTabla")["Fitness"].median().sort_values(ascending=False).index
    
    sns.boxplot(data=df, x="AlgoritmoTabla", y="Fitness", order=orden)
    plt.xlabel("Algoritmos")
    plt.ylabel("Fitness")
    plt.title("Distribución de Fitness por Algoritmo", fontsize=16, fontweight='bold')
    plt.xticks(rotation=45, ha='right')
    
    plt.tight_layout()
    plt.savefig("./output/graficas/distribucion_fitness.png", dpi=300, bbox_inches='tight')
    plt.show()



def grafica_eficiencia_tiempo(df):
    """Gráfica de eficiencia (fitness/tiempo)"""
    plt.figure(figsize=(14, 8))
    
    # Calcular eficiencia
    df_eficiencia = df.copy()
    df_eficiencia["Eficiencia"] = df_eficiencia["Fitness"] / (df_eficiencia["Time"] + 0.001)  # Evitar división por 0
    
    # Promedio por algoritmo
    eficiencia_prom = df_eficiencia.groupby("AlgoritmoTabla")["Eficiencia"].mean().sort_values(ascending=False)
    
    bars = plt.bar(range(len(eficiencia_prom)), eficiencia_prom.values,
                   color=sns.color_palette("plasma", len(eficiencia_prom)))
    
    plt.xlabel("Algoritmos")
    plt.ylabel("Eficiencia (Fitness/Tiempo)")
    plt.title("Eficiencia de Algoritmos (Fitness por unidad de tiempo)", fontsize=16, fontweight='bold')
    plt.xticks(range(len(eficiencia_prom)), eficiencia_prom.index, rotation=45, ha='right')
    
    # Añadir valores
    for i, bar in enumerate(bars):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                f'{height:.1f}', ha='center', va='bottom', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig("./output/graficas/eficiencia_algoritmos.png", dpi=300, bbox_inches='tight')
    plt.show()



def resumen_estadistico(df):
    """Genera un resumen estadístico y lo guarda"""
    print("\n" + "="*50)
    print("RESUMEN ESTADÍSTICO DE LOS ALGORITMOS")
    print("="*50)
    
    # Estadísticas por algoritmo
    stats = df.groupby("AlgoritmoTabla").agg({
        "Fitness": ["mean", "std", "min", "max"],
        "Time": ["mean", "std"],
        "Evals": ["mean", "std"],
        "Position": ["mean", "std"]
    }).round(3)
    
    print("\nEstadísticas detalladas:")
    print(stats.to_string())
    
    # Guardar en CSV
    stats.to_csv("./output/graficas/estadisticas_detalladas.csv")
    
    # Mejor algoritmo en cada métrica
    print(f"\n🥇 MEJORES POR MÉTRICA:")
    print(f"Mejor Fitness Promedio: {df.groupby('AlgoritmoTabla')['Fitness'].mean().idxmax()}")
    print(f"Mejor Posición Promedio: {df.groupby('AlgoritmoTabla')['Position'].mean().idxmin()}")
    print(f"Más Rápido: {df.groupby('AlgoritmoTabla')['Time'].mean().idxmin()}")
    print(f"Menos Evaluaciones: {df.groupby('AlgoritmoTabla')['Evals'].mean().idxmin()}")

def main():
    """Función principal"""
    print("Iniciando generación de gráficas...")
    
    # Crear directorio
    crear_directorio_graficas()
    
    # Procesar datos
    df = procesar_datos()
    
    print(f"Datos procesados: {len(df)} registros, {df['AlgoritmoTabla'].nunique()} algoritmos")
    print("Generando gráficas...")
    
    # Generar las gráficas seleccionadas
    grafica_fitness_por_algoritmo(df)
    grafica_posicion_promedio(df)
    grafica_boxplot_fitness(df)
    grafica_eficiencia_tiempo(df)
    
    # Resumen estadístico
    resumen_estadistico(df)
    
    print("\n✅ ¡Todas las gráficas han sido generadas!")
    print("📁 Revisa la carpeta './output/graficas/' para ver los resultados")

if __name__ == "__main__":
    main()