#include "ils.h"
#include <random.hpp>
#include <algorithm>

ResultMH ILS::optimize(Problem* problem, int /*maxEvals*/) {
    const int iterations = 10;
    const int blMaxEvals = 100;

    LocalSearch ls(SearchStrategy::BLsmall);

    // 1) Solución inicial aleatoria y BLsmall
    tSolution bestSol = problem->createSolution();
    tFitness bestFit = problem->fitness(bestSol);
    int totalEvals = 1;

    ResultMH res = ls.optimize(problem, blMaxEvals);
    bestSol = res.solution;
    bestFit = res.fitness;
    totalEvals += res.evaluations;

    size_t m = bestSol.size();
    int numToMutate = std::max(2, int(0.2 * m));

    // 2) Iteraciones restantes
    for (int it = 1; it < iterations; ++it) {
        // Generar mutante a partir de bestSol
        tSolution mutant = bestSol;

        // Construir lista de no seleccionados
        std::vector<bool> inSol(problem->getSolutionSize(), false);
        for (int v : mutant) inSol[v] = true;
        std::vector<int> nonSel;
        nonSel.reserve(problem->getSolutionSize() - m);
        for (int i = 0; i < (int)problem->getSolutionSize(); ++i)
            if (!inSol[i]) nonSel.push_back(i);

        // Elegir posiciones a mutar
        std::vector<int> positions(m);
        for (size_t i = 0; i < m; ++i) positions[i] = i;
        Random::shuffle(positions.begin(), positions.end());

        // Mutar numToMutate posiciones
        for (int k = 0; k < numToMutate; ++k) {
            int pos = positions[k];
            int idxNon = Random::get<int>(0, (int)nonSel.size() - 1);
            mutant[pos] = nonSel[idxNon];
            nonSel[idxNon] = bestSol[pos];  // actualizar nonSel
        }

        // Optimizar mutante con BLsmall
        ResultMH r2 = ls.optimize(problem, blMaxEvals);
        totalEvals += r2.evaluations;

        // Actualizar mejor
        if (r2.fitness > bestFit) {
            bestFit = r2.fitness;
            bestSol = r2.solution;
        }
    }

    return ResultMH(bestSol, bestFit, totalEvals);
}
