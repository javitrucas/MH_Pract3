#include <cassert>
#include <limits>
#include <randomsearch.h>

using namespace std;

/**
 * Create random solutions until maxevals has been achieved, and returns the
 * best one (minimizing fitness, which in SNIMP corresponde a maximizar spread).
 *
 * @param problem The problem to be optimized
 * @param maxevals Maximum number of evaluations allowed
 * @return A ResultMH containing the best solution found and its fitness
 */
ResultMH RandomSearch::optimize(Problem *problem, int maxevals) {
    assert(maxevals > 0);
    tSolution bestSol;
    // En SNIMP, fitness = -spread, así que buscamos el mínimo
    tFitness bestFit = numeric_limits<tFitness>::max();

    for (int eval = 0; eval < maxevals; ++eval) {
        tSolution sol = problem->createSolution();
        tFitness fit = problem->fitness(sol);
        if (fit < bestFit) {
            bestFit = fit;
            bestSol = sol;
        }
    }

    return ResultMH(bestSol, bestFit, maxevals);
}
