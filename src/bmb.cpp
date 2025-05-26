#include "bmb.h"
#include "pincrem.h"  // Para ProblemIncrem

ResultMH BMB::optimize(Problem* problem, int /*maxEvals*/) {
    const int restarts = 10;
    const int blMaxEvals = 100;

    // Búsqueda Local con criterio BLsmall (max 20 sin mejora o blMaxEvals)
    LocalSearch ls(SearchStrategy::BLsmall);

    tSolution bestSol;
    tFitness bestFit = std::numeric_limits<tFitness>::lowest();
    int totalEvals = 0;

    for (int i = 0; i < restarts; ++i) {
        // 1. Solución aleatoria
        tSolution sol = problem->createSolution();
        tFitness fit = problem->fitness(sol);
        totalEvals += 1;

        // 2. Optimizar con BLsmall
        ResultMH res = ls.optimize(problem, blMaxEvals);
        totalEvals += res.evaluations;

        // 3. Actualizar mejor solución global
        if (res.fitness > bestFit) {
            bestFit = res.fitness;
            bestSol = res.solution;
        }
    }

    return ResultMH(bestSol, bestFit, totalEvals);
}
