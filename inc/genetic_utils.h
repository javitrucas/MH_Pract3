#ifndef GENETIC_UTILS_H
#define GENETIC_UTILS_H

#include "problem.h"    // Problem, tSolution, tFitness
#include <vector>
#include <utility>

/// Agrupa una solución y su fitness
struct Individual {
    tSolution sol;
    tFitness fitness;
};

class GeneticUtils {
protected:
    // Selección por torneo (maximiza fitness)
    Individual tournamentSelect(const std::vector<Individual>& pop, int k = 3) const;

    // Crossover sin orden (con reparación)
    std::pair<tSolution, tSolution>
    crossoverSinOrden(const tSolution& p1, const tSolution& p2) const;

    // Crossover con orden (respeta orden sin repetidos)
    std::pair<tSolution, tSolution>
    crossoverConOrden(const tSolution& p1, const tSolution& p2) const;

    // Mutación por intercambio de dos posiciones
    void mutate(std::vector<int>& sol) const;
};

#endif // GENETIC_UTILS_H
