import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.patches import Rectangle
import warnings
warnings.filterwarnings('ignore')

# Configuración de estilo
plt.style.use('default')
sns.set_palette("husl")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

def load_and_clean_data(filepath):
    """Carga y limpia los datos del CSV"""
    df = pd.read_csv(filepath)
    
    # Limpiar nombres de instancias
    df['Instancia_clean'] = df['Instancia'].str.replace('../datos/', '').str.replace('.txt', '')
    
    # Calcular eficiencia (fitness/tiempo)
    df['Eficiencia'] = df['Fitness'] / df['Time']
    
    # Categorizar algoritmos
    def categorize_algorithm(alg):
        if 'RandomSearch' in alg:
            return 'Búsqueda Aleatoria'
        elif 'Greedy' in alg:
            return 'Algoritmos Greedy'
        elif 'LocalSearch' in alg or 'LS' in alg:
            return 'Búsqueda Local'
        elif any(x in alg for x in ['AGG', 'AGE']):
            return 'Algoritmos Genéticos'
        elif 'AM' in alg:
            return 'Algoritmos Meméticos'
        elif any(x in alg for x in ['ES', 'BMB', 'ILS']):
            return 'Metaheurísticas'
        elif 'GRASP' in alg:
            return 'GRASP'
        else:
            return 'Otros'
    
    df['Categoria'] = df['Algoritmo'].apply(categorize_algorithm)
    
    return df

def plot_fitness_comparison(df):
    """Gráfico de comparación de fitness por algoritmo e instancia"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('Análisis Comparativo de Algoritmos de Optimización', fontsize=16, fontweight='bold')
    
    # 1. Box plot de fitness por categoría
    ax1 = axes[0, 0]
    sns.boxplot(data=df, x='Categoria', y='Fitness', ax=ax1)
    ax1.set_title('Distribución de Fitness por Categoría de Algoritmo')
    ax1.set_xlabel('Categoría de Algoritmo')
    ax1.set_ylabel('Fitness')
    ax1.tick_params(axis='x', rotation=45)
    
    # 2. Heatmap de fitness promedio
    ax2 = axes[0, 1]
    pivot_data = df.groupby(['Instancia_clean', 'Categoria'])['Fitness'].mean().unstack()
    sns.heatmap(pivot_data, annot=True, fmt='.2f', cmap='RdYlBu_r', ax=ax2)
    ax2.set_title('Fitness Promedio por Instancia y Categoría')
    ax2.set_xlabel('Categoría de Algoritmo')
    ax2.set_ylabel('Instancia')
    
    # 3. Scatter plot: Tiempo vs Fitness
    ax3 = axes[1, 0]
    for categoria in df['Categoria'].unique():
        subset = df[df['Categoria'] == categoria]
        ax3.scatter(subset['Time'], subset['Fitness'], label=categoria, alpha=0.7, s=60)
    ax3.set_xlabel('Tiempo de Ejecución (s)')
    ax3.set_ylabel('Fitness')
    ax3.set_title('Relación Tiempo vs Fitness')
    ax3.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    ax3.set_xscale('log')
    
    # 4. Eficiencia por algoritmo
    ax4 = axes[1, 1]
    efficiency_data = df.groupby('Categoria')['Eficiencia'].agg(['mean', 'std']).reset_index()
    bars = ax4.bar(efficiency_data['Categoria'], efficiency_data['mean'], 
                   yerr=efficiency_data['std'], capsize=5, alpha=0.8)
    ax4.set_title('Eficiencia Promedio por Categoría (Fitness/Tiempo)')
    ax4.set_xlabel('Categoría de Algoritmo')
    ax4.set_ylabel('Eficiencia')
    ax4.tick_params(axis='x', rotation=45)
    
    # Añadir valores en las barras
    for bar, val in zip(bars, efficiency_data['mean']):
        ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.1,
                f'{val:.1f}', ha='center', va='bottom')
    
    plt.tight_layout()
    return fig

def plot_detailed_analysis(df):
    """Análisis detallado de los mejores algoritmos"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('Análisis Detallado de Rendimiento de Algoritmos', fontsize=16, fontweight='bold')
    
    # 1. Top 10 algoritmos por fitness
    ax1 = axes[0, 0]
    top_algorithms = df.nlargest(10, 'Fitness')
    bars1 = ax1.barh(range(len(top_algorithms)), top_algorithms['Fitness'])
    ax1.set_yticks(range(len(top_algorithms)))
    ax1.set_yticklabels(top_algorithms['Algoritmo'], fontsize=8)
    ax1.set_xlabel('Fitness')
    ax1.set_title('Top 10 Algoritmos por Fitness')
    
    # Colorear barras por categoría
    colors = plt.cm.Set3(np.linspace(0, 1, len(top_algorithms['Categoria'].unique())))
    color_map = dict(zip(top_algorithms['Categoria'].unique(), colors))
    for i, (bar, cat) in enumerate(zip(bars1, top_algorithms['Categoria'])):
        bar.set_color(color_map[cat])
    
    # 2. Evaluaciones vs Fitness
    ax2 = axes[0, 1]
    scatter = ax2.scatter(df['Evals'], df['Fitness'], c=df['Time'], 
                         cmap='viridis', alpha=0.7, s=60)
    ax2.set_xlabel('Número de Evaluaciones')
    ax2.set_ylabel('Fitness')
    ax2.set_title('Evaluaciones vs Fitness (Color = Tiempo)')
    ax2.set_xscale('log')
    plt.colorbar(scatter, ax=ax2, label='Tiempo (s)')
    
    # 3. Comparación por instancia
    ax3 = axes[1, 0]
    instance_stats = df.groupby('Instancia_clean').agg({
        'Fitness': ['mean', 'max', 'min'],
        'Time': 'mean'
    }).round(2)
    
    x_pos = np.arange(len(instance_stats))
    ax3.bar(x_pos - 0.2, instance_stats[('Fitness', 'max')], 0.4, 
           label='Máximo', alpha=0.8)
    ax3.bar(x_pos + 0.2, instance_stats[('Fitness', 'mean')], 0.4, 
           label='Promedio', alpha=0.8)
    ax3.set_xlabel('Instancia')
    ax3.set_ylabel('Fitness')
    ax3.set_title('Rendimiento por Instancia')
    ax3.set_xticks(x_pos)
    ax3.set_xticklabels(instance_stats.index, rotation=45)
    ax3.legend()
    
    # 4. Análisis de convergencia (Evals vs Tiempo)
    ax4 = axes[1, 1]
    for categoria in df['Categoria'].unique():
        subset = df[df['Categoria'] == categoria]
        ax4.scatter(subset['Evals'], subset['Time'], label=categoria, alpha=0.7, s=60)
    ax4.set_xlabel('Número de Evaluaciones')
    ax4.set_ylabel('Tiempo de Ejecución (s)')
    ax4.set_title('Convergencia: Evaluaciones vs Tiempo')
    ax4.set_xscale('log')
    ax4.set_yscale('log')
    ax4.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout()
    return fig

def plot_pareto_analysis(df):
    """Análisis de Pareto: Fitness vs Tiempo"""
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    fig.suptitle('Análisis de Frontera de Pareto: Fitness vs Tiempo', fontsize=16, fontweight='bold')
    
    # 1. Scatter plot con frontera de Pareto
    ax1 = axes[0]
    
    # Calcular frontera de Pareto (maximizar fitness, minimizar tiempo)
    def is_pareto_optimal(costs):
        is_efficient = np.ones(costs.shape[0], dtype=bool)
        for i, c in enumerate(costs):
            if is_efficient[i]:
                # Punto es dominado si otro punto tiene mejor fitness Y mejor tiempo
                is_efficient[is_efficient] = np.any(
                    (costs[is_efficient] >= c) & 
                    (costs[is_efficient] != c).any(axis=1), axis=1)
                is_efficient[i] = True
        return is_efficient
    
    # Preparar datos para Pareto (invertir tiempo para minimización)
    pareto_data = np.column_stack([df['Fitness'].values, -df['Time'].values])
    pareto_mask = is_pareto_optimal(pareto_data)
    
    # Plot todos los puntos
    for categoria in df['Categoria'].unique():
        subset = df[df['Categoria'] == categoria]
        ax1.scatter(subset['Time'], subset['Fitness'], label=categoria, alpha=0.6, s=50)
    
    # Destacar puntos de Pareto
    pareto_points = df[pareto_mask]
    ax1.scatter(pareto_points['Time'], pareto_points['Fitness'], 
               color='red', s=100, marker='*', label='Pareto Óptimo', 
               edgecolor='black', linewidth=1)
    
    ax1.set_xlabel('Tiempo de Ejecución (s)')
    ax1.set_ylabel('Fitness')
    ax1.set_title('Frontera de Pareto: Fitness vs Tiempo')
    ax1.set_xscale('log')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Ranking de algoritmos por dominancia
    ax2 = axes[1]
    
    # Contar cuántos puntos domina cada algoritmo
    dominance_score = []
    for i, row in df.iterrows():
        dominated = ((df['Fitness'] <= row['Fitness']) & (df['Time'] >= row['Time'])).sum() - 1
        dominance_score.append(dominated)
    
    df['Dominance_Score'] = dominance_score
    top_dominant = df.nlargest(15, 'Dominance_Score')
    
    bars = ax2.barh(range(len(top_dominant)), top_dominant['Dominance_Score'])
    ax2.set_yticks(range(len(top_dominant)))
    ax2.set_yticklabels(top_dominant['Algoritmo'], fontsize=8)
    ax2.set_xlabel('Puntos Dominados')
    ax2.set_title('Ranking de Dominancia de Algoritmos')
    
    # Colorear por categoría
    colors = plt.cm.Set3(np.linspace(0, 1, len(top_dominant['Categoria'].unique())))
    color_map = dict(zip(top_dominant['Categoria'].unique(), colors))
    for bar, cat in zip(bars, top_dominant['Categoria']):
        bar.set_color(color_map[cat])
    
    plt.tight_layout()
    return fig

def generate_summary_report(df):
    """Genera un reporte resumen de los resultados"""
    print("=" * 80)
    print("REPORTE RESUMEN DE ANÁLISIS DE ALGORITMOS DE OPTIMIZACIÓN")
    print("=" * 80)
    
    print(f"\n📊 ESTADÍSTICAS GENERALES:")
    print(f"• Total de experimentos: {len(df)}")
    print(f"• Algoritmos únicos: {df['Algoritmo'].nunique()}")
    print(f"• Instancias evaluadas: {df['Instancia_clean'].nunique()}")
    print(f"• Categorías de algoritmos: {df['Categoria'].nunique()}")
    
    print(f"\n🏆 MEJORES RESULTADOS:")
    best_fitness = df.loc[df['Fitness'].idxmax()]
    print(f"• Mejor fitness: {best_fitness['Fitness']:.2f}")
    print(f"  - Algoritmo: {best_fitness['Algoritmo']}")
    print(f"  - Instancia: {best_fitness['Instancia_clean']}")
    print(f"  - Tiempo: {best_fitness['Time']:.4f}s")
    
    fastest = df.loc[df['Time'].idxmin()]
    print(f"• Más rápido: {fastest['Time']:.6f}s")
    print(f"  - Algoritmo: {fastest['Algoritmo']}")
    print(f"  - Fitness: {fastest['Fitness']:.2f}")
    
    most_efficient = df.loc[df['Eficiencia'].idxmax()]
    print(f"• Más eficiente: {most_efficient['Eficiencia']:.2f}")
    print(f"  - Algoritmo: {most_efficient['Algoritmo']}")
    
    print(f"\n📈 ANÁLISIS POR CATEGORÍA:")
    category_stats = df.groupby('Categoria').agg({
        'Fitness': ['mean', 'max', 'std'],
        'Time': ['mean', 'min'],
        'Eficiencia': 'mean'
    }).round(3)
    
    for categoria in category_stats.index:
        print(f"\n• {categoria}:")
        print(f"  - Fitness promedio: {category_stats.loc[categoria, ('Fitness', 'mean')]:.2f}")
        print(f"  - Fitness máximo: {category_stats.loc[categoria, ('Fitness', 'max')]:.2f}")
        print(f"  - Tiempo promedio: {category_stats.loc[categoria, ('Time', 'mean')]:.4f}s")
        print(f"  - Eficiencia promedio: {category_stats.loc[categoria, ('Eficiencia', 'mean')]:.2f}")
    
    print(f"\n🎯 RECOMENDACIONES:")
    print("• Para máximo fitness: Usar algoritmos GRASP o Greedy")
    print("• Para mínimo tiempo: Usar algoritmos ES (Evolution Strategy)")
    print("• Para mejor balance: Considerar algoritmos con alta eficiencia")
    print("• Los algoritmos meméticos muestran buen compromiso tiempo-calidad")

def main():
    """Función principal"""
    # Cargar datos
    filepath = './output/results.csv'  # Ajusta la ruta según necesites
    try:
        df = load_and_clean_data(filepath)
        print("Datos cargados exitosamente!")
        print(f"Dimensiones del dataset: {df.shape}")
        
        # Generar visualizaciones
        print("\n🎨 Generando visualizaciones...")
        
        # 1. Comparación general
        fig1 = plot_fitness_comparison(df)
        plt.show()
        
        # 2. Análisis detallado
        fig2 = plot_detailed_analysis(df)
        plt.show()
        
        # 3. Análisis de Pareto
        fig3 = plot_pareto_analysis(df)
        plt.show()
        
        # 4. Reporte resumen
        generate_summary_report(df)
        
        print("\n✅ Análisis completado!")
        
        # Opcional: Guardar figuras
        save_figs = input("\n¿Deseas guardar las figuras? (y/n): ").lower().strip()
        if save_figs == 'y':
            fig1.savefig('analisis_comparativo.png', dpi=300, bbox_inches='tight')
            fig2.savefig('analisis_detallado.png', dpi=300, bbox_inches='tight')
            fig3.savefig('analisis_pareto.png', dpi=300, bbox_inches='tight')
            print("📁 Figuras guardadas como PNG")
        
    except FileNotFoundError:
        print("❌ Error: No se pudo encontrar el archivo results.csv")
        print("Asegúrate de que la ruta sea correcta")
    except Exception as e:
        print(f"❌ Error inesperado: {e}")

if __name__ == "__main__":
    main()