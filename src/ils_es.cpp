// src/ils_es.cpp

#include "ils_es.h"
#include "pincrem.h"
#include <random.hpp>
#include <algorithm>
#include <limits>
#include <vector>
#include <numeric>

using namespace std;

ResultMH ILSES::optimize(Problem* problem, int /*maxEvals*/) {
    const int iterations = 10;
    const int esMaxEvals = 100;

    ES es;  // Simulated Annealing

    // 1) Solución inicial y ES
    tSolution bestSol = problem->createSolution();
    tFitness bestFit = problem->fitness(bestSol);
    int totalEvals = 1;

    ResultMH res = es.optimize(problem, esMaxEvals);
    bestSol = res.solution;
    bestFit = res.fitness;
    totalEvals += res.evaluations;

    size_t m = bestSol.size();
    int numToMutate = max(2, int(0.2 * m));

    // 2) Iteraciones restantes
    for (int it = 1; it < iterations; ++it) {
        // Mutación del 20% de componentes
        tSolution mutant = bestSol;
        std::vector<bool> inSol(problem->getSolutionSize(), false);
        for (int v : mutant) inSol[v] = true;

        std::vector<int> nonSel;
        nonSel.reserve(problem->getSolutionSize() - m);
        for (int i = 0; i < (int)problem->getSolutionSize(); ++i)
            if (!inSol[i]) nonSel.push_back(i);

        std::vector<int> positions(m);
        iota(positions.begin(), positions.end(), 0);
        Random::shuffle(positions.begin(), positions.end());

        for (int k = 0; k < numToMutate; ++k) {
            int pos = positions[k];
            int idxNon = Random::get<int>(0, (int)nonSel.size() - 1);
            int newVal = nonSel[idxNon];
            nonSel[idxNon] = mutant[pos];
            mutant[pos] = newVal;
        }

        // Aplicar ES a la solución mutada
        ProblemIncrem* realP = dynamic_cast<ProblemIncrem*>(problem);
        realP->size = problem->getSolutionSize(); // ensure size
        // Aquí falta usar la mutación como estado inicial para ES...
        ResultMH r2 = es.optimize(problem, esMaxEvals);
        totalEvals += r2.evaluations;

        if (r2.fitness > bestFit) {
            bestFit = r2.fitness;
            bestSol = r2.solution;
        }
    }

    return ResultMH(bestSol, bestFit, totalEvals);
}
