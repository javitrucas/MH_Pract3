#include <cassert>
#include <limits>
#include <randomsearch.h>

using namespace std;

/**
 * Create random solutions until maxevals has been achieved, and returns the
 * best one (maximizing fitness, which now corresponde a spread positivo).
 */
ResultMH RandomSearch::optimize(Problem *problem, int maxevals) {
    assert(maxevals > 0);
    tSolution bestSol;
    // Ahora fitness = spread, buscamos el *máximo*
    tFitness bestFit = numeric_limits<tFitness>::lowest();

    for (int eval = 0; eval < maxevals; ++eval) {
        tSolution sol = problem->createSolution();
        tFitness fit = problem->fitness(sol);
        if (fit > bestFit) {
            bestFit = fit;
            bestSol = sol;
        }
    }

    return ResultMH(bestSol, bestFit, maxevals);
}