#pragma once
#include <vector>
#include <string>
#include <problem.h>

/**
 * Clase que almacena información para evaluación factorada.
 */
class ProblemIncremFactoringInfo : public SolutionFactoringInfo {
public:
    std::vector<tFitness> fitness_parcial; // Valores parciales de fitness

    ProblemIncremFactoringInfo(size_t m) : fitness_parcial(m, 0) {}
    ProblemIncremFactoringInfo() {}
};

class ProblemIncrem : public Problem {
public:
    size_t size;                          // Número de nodos en el grafo
    int m;                                // Número de seeds a seleccionar
    double p_icm;                         // Probabilidad de activación
    int ev_icm;                           // Número de simulaciones Monte Carlo
    std::vector<std::vector<int>> adj;    // Lista de adyacencia

    ProblemIncrem(size_t size = 0)
        : Problem(), size(size), m(0), p_icm(0.01), ev_icm(10) {}

    // Carga grafo SNAP
    void leerArchivo(const std::string& filename);

    // Crea solución aleatoria (seed set)
    tSolution createSolution() override;

    // Fitness = -spread (se minimiza)
    tFitness fitness(const tSolution &solution) override;

    // Tamaño y rango de dominio
    size_t getSolutionSize() override { return size; }
    std::pair<tDomain, tDomain> getSolutionDomainRange() override {
        return std::make_pair(static_cast<tDomain>(0), static_cast<tDomain>(size - 1));
    }
    int getM() const { return m; }

    // Evaluación factorada (no usada)
    SolutionFactoringInfo* generateFactoringInfo(const tSolution &solution) override;
    void updateSolutionFactoringInfo(SolutionFactoringInfo *sol_info,
                                     const tSolution &solution,
                                     unsigned pos_change,
                                     tDomain new_value) override;

private:
    // Simulación Independent Cascade (ICM)
    double simulateICM(const tSolution &seed_set);
};
