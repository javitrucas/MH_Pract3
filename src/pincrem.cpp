#include <algorithm>
#include <pincrem.h>
#include <random.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <numeric>
#include <utility>

// Helper para asegurar orden de índices (a <= b)
static inline std::pair<int,int> orderedPair(int a, int b) {
    return (a <= b) ? std::make_pair(a,b) : std::make_pair(b,a);
}

// Calcula la diferencia entre el nodo más conectado y el menos conectado
tFitness ProblemIncrem::fitness(const tSolution &solution) {
    // Construimos un vector con la suma de conexiones de cada nodo
    size_t n = solution.size();
    std::vector<tFitness> sums(n, 0);
    
    // Recorremos solo combinaciones i<j para eficiencia y actualizamos ambos extremos
    for (size_t i = 0; i + 1 < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            auto [u,v] = orderedPair(solution[i], solution[j]);
            if (u >= 0 && v < size) {
                tFitness w = D[u][v];
                sums[i] += w;
                sums[j] += w;
            } else {
                std::cerr << "Índices inválidos en fitness: " << u << "," << v << std::endl;
            }
        }
    }
    
    // Calculamos máximo y mínimo
    auto [minIt, maxIt] = std::minmax_element(sums.begin(), sums.end());
    return (*maxIt - *minIt);
}

// Genera una solución aleatoria válida con m elementos únicos
tSolution ProblemIncrem::createSolution() {
    // Creamos y mezclamos un vector de 0..size-1
    std::vector<int> pool(size);
    std::iota(pool.begin(), pool.end(), 0);
    for (size_t i = size - 1; i > 0; --i) {
        size_t j = Random::get<size_t>(0, i);
        std::swap(pool[i], pool[j]);
    }
    
    // Tomamos los primeros m elementos y ordenamos
    tSolution sol(pool.begin(), pool.begin() + m);
    std::sort(sol.begin(), sol.end());
    return sol;
}

// Lee el archivo con la matriz de distancias o pesos
void ProblemIncrem::leerArchivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir: " + filename);
    }
    
    // Primero filas, columnas y reservamos
    file >> size >> m;
    D.assign(size, std::vector<float>(size, 0.0f));

    int i, j;
    double valor;
    while (file >> i >> j >> valor) {
        if (i >= 0 && j >= 0 && i < size && j < size) {
            D[i][j] = static_cast<float>(valor);
        }
    }
}

// Calcula la información de factorización para cada nodo de la solución
SolutionFactoringInfo *ProblemIncrem::generateFactoringInfo(const tSolution &solution) {
    auto *info = new ProblemIncremFactoringInfo(m);
    
    for (size_t i = 0; i < solution.size(); ++i) {
        // Usamos std::accumulate sobre los demás
        info->fitness_parcial[i] = std::accumulate(
            solution.begin(), solution.end(), 0.0f,
            [&](float acc, int other) {
                if (other == solution[i]) return acc;
                auto [u,v] = orderedPair(solution[i], other);
                return acc + D[u][v];
            }
        );
    }
    return info;
}

// Actualiza solo la parte afectada del fitness tras un cambio en la solución
void ProblemIncrem::updateSolutionFactoringInfo(SolutionFactoringInfo *solution_info,
                                                const tSolution &solution,
                                                unsigned pos_change,
                                                tDomain new_value) {
    auto *info = static_cast<ProblemIncremFactoringInfo*>(solution_info);
    int old_value = solution[pos_change];
    
    // Recalculamos fitness_parcial[pos_change] desde cero
    info->fitness_parcial[pos_change] = std::accumulate(
        solution.begin(), solution.end(), 0.0f,
        [&](float acc, int other) {
            if (&other == &solution[pos_change]) return acc;
            int val = (other == old_value) ? new_value : other;
            auto [u,v] = orderedPair(solution[pos_change], val);
            return acc + D[u][v];
        }
    );
}
