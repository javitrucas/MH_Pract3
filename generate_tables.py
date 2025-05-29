import pandas as pd
import re

def renombrar_algoritmo(alg):
    if alg.startswith("Mejor-P2("):
        return "Mejor Práctica 2"
    return alg

# Carga el CSV
df = pd.read_csv("./output/results.csv")

# Limpieza de nombres de instancia
df["Instancia"] = df["Instancia"].apply(lambda x: x.split("/")[-1])

# Renombramos la columna original para no perderla
df["AlgOriginal"] = df["Algoritmo"]
# Aplicamos el renombrado genérico de Mejor-P2(...)
df["Algoritmo"] = df["Algoritmo"].apply(renombrar_algoritmo)

# PASO ADICIONAL: calcular BL como el mejor entre LSall y BLsmall
bl_rows = []
for inst, grupo in df.groupby("Instancia"):
    # Filtramos las dos variantes de LocalSearch
    ls = grupo[grupo["AlgOriginal"].isin(["LocalSearch-LSall", "LocalSearch-BLsmall"])]
    if not ls.empty:
        # Seleccionamos la fila con mejor fitness
        mejor = ls.loc[ls["Fitness"].idxmax()].copy()
        mejor["Algoritmo"] = "BL"
        bl_rows.append(mejor)

# Convertimos la lista de BL a DataFrame
df_bl = pd.DataFrame(bl_rows)

# Eliminamos todas las filas de LocalSearch-* originales
df = df[~df["AlgOriginal"].isin(["LocalSearch-LSall", "LocalSearch-BLsmall"])]

# Añadimos las filas BL
df = pd.concat([df, df_bl], ignore_index=True)

# Mapeo de nombres de algoritmo al nombre solicitado en la tabla
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

# Orden deseado en la tabla
orden_algoritmos = [
    "Random", "Greedy", "BL", "Mejor Práctica 2", "BMB",
    "ILS", "ES", "ILS-ES", "GRASP-NOBL", "GRASP-SIBL"
]

# Filtramos solo los algoritmos que nos interesan y aplicamos el mapeo final
df = df[df["Algoritmo"].isin(nombre_tabla.keys())].copy()
df["AlgoritmoTabla"] = df["Algoritmo"].map(nombre_tabla)

# Generar tablas por instancia
tablas = {}
for instancia, grupo in df.groupby("Instancia"):
    tabla = grupo.set_index("AlgoritmoTabla").reindex(orden_algoritmos)
    tabla_final = tabla[["Position", "Fitness", "Time", "Evals"]].reset_index()
    tablas[instancia] = tabla_final
    print(f"\nTabla de resultados para el conjunto {instancia}:\n")
    print(tabla_final.to_string(index=False))

# Guardar CSVs si se desea
for instancia, tabla in tablas.items():
    tabla.to_csv(f"./output/tables/{instancia}.csv", index=False)
