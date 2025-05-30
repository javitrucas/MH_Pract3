import pandas as pd

def renombrar_algoritmo(alg):
    if alg.startswith("Mejor-P2("):
        return "Mejor Práctica 2"
    return alg

# Carga el CSV
df = pd.read_csv("./output/results.csv")

# Limpieza de nombres de instancia
df["Instancia"] = df["Instancia"].apply(lambda x: x.split("/")[-1])

# Conservamos el nombre original para BL
df["AlgOriginal"] = df["Algoritmo"]
df["Algoritmo"] = df["Algoritmo"].apply(renombrar_algoritmo)

# —— 1) Construir la fila BL como antes ——
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

# —— 2) Filtrar sólo los algoritmos deseados ——
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
orden_algoritmos = [
    "Random", "Greedy", "BL", "Mejor Práctica 2", "BMB",
    "ILS", "ES", "ILS-ES", "GRASP-NOBL", "GRASP-SIBL"
]

df = df[df["Algoritmo"].isin(nombre_tabla.keys())].copy()
df["AlgoritmoTabla"] = df["Algoritmo"].map(nombre_tabla)

# —— 3) Recalcular posiciones por instancia ordenando fitness descendente ——
df["Position"] = (
    df
    .groupby("Instancia")["Fitness"]
    .rank(method="first", ascending=False)
    .astype(int)
)

# —— 4) Generar y mostrar tablas por conjunto —— 
for instancia, grupo in df.groupby("Instancia"):
    tabla = (
        grupo
        .set_index("AlgoritmoTabla")
        .reindex(orden_algoritmos)
    )
    tabla_final = tabla[["Position", "Fitness", "Time", "Evals"]].reset_index()
    print(f"\nTabla de resultados para el conjunto {instancia}:\n")
    print(tabla_final.to_string(index=False))
    tabla_final.to_csv(f"./output/tables/{instancia}.csv", index=False)

# —— 5) Tabla agregada final —— 
tabla_agregada = (
    df.groupby("AlgoritmoTabla")
    .agg({
        "Position": "mean",
        "Time": "mean",
        "Evals": "mean"
    })
    .reindex(orden_algoritmos)
    .round(2)
    .rename(columns={
        "Position": "Posición Promedio",
        "Time": "Tiempo Promedio (segs)",
        "Evals": "Evaluaciones Promedio"
    })
    .reset_index()
)

print("\nTabla final de resultados (promedios entre instancias):\n")
print(tabla_agregada.to_string(index=False))
tabla_agregada.to_csv("./output/tables/final_summary.csv", index=False)
