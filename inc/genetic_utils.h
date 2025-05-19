#ifndef GENETIC_UTILS_H
#define GENETIC_UTILS_H

#include "problem.h"    // Problem, tSolution, tFitness
#include "pincrem.h"    // ProblemIncrem
#include <vector>
#include <utility>

/// Agrupa una solución y su fitness
struct Individual {
    tSolution sol;
    tFitness fitness;
};

class GeneticUtils {
protected:
    std::vector<int> list2bin(const tSolution& lst, int n) const;
    tSolution bin2list(const std::vector<int>& bin) const;
    Individual tournamentSelect(const std::vector<Individual>& pop, int k = 3) const;

    std::pair<std::vector<int>, std::vector<int>>
    crossoverUniformBin(const std::vector<int>& b1,
                        const std::vector<int>& b2,
                        int targetOnes) const;

    std::pair<std::vector<int>, std::vector<int>>
    crossoverPositionBin(const std::vector<int>& b1,
                         const std::vector<int>& b2) const;

    void mutateBin(std::vector<int>& bin) const;
};

#endif // GENETIC_UTILS_H
