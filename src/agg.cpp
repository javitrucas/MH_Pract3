#include "agg.h"
#include <algorithm>
#include <iostream>

using namespace std;

AGG::AGG(int popSize, double pc, double pm)
  : popSize_(popSize)
  , pc_(pc)
  , pm_(pm)
  , crossoverOp_(AGGCrossover::CON_ORDEN)
{ }

ResultMH AGG::optimize(Problem* problem, int maxEvals) {
    // 1) Inicializar población
    vector<Individual> population;
    population.reserve(popSize_);
    for (int i = 0; i < popSize_; ++i) {
        Individual ind;
        ind.sol     = problem->createSolution();
        ind.fitness = problem->fitness(ind.sol);
        population.push_back(ind);
    }
    int evals = popSize_;

    // 2) Bucle generacional
    while (evals < maxEvals) {
        // 2.1) Selección por torneo
        vector<Individual> parents;
        parents.reserve(popSize_);
        for (int i = 0; i < popSize_; ++i)
            parents.push_back(tournamentSelect(population, 3));

        // 2.2) Cruce y mutación
        vector<Individual> children;
        children.reserve(popSize_);

        for (int i = 0; i < popSize_; i += 2) {
            auto& p1 = parents[i];
            auto& p2 = parents[i+1];

            tSolution c1 = p1.sol, c2 = p2.sol;
            if (Random::get<double>(0.0,1.0) < pc_) {
                if (crossoverOp_ == AGGCrossover::CON_ORDEN) {
                    tie(c1, c2) = crossoverConOrden(p1.sol, p2.sol);
                } else {
                    tie(c1, c2) = crossoverSinOrden(p1.sol, p2.sol);
                }
            }

            if (Random::get<double>(0.0,1.0) < pm_) mutate(c1);
            if (Random::get<double>(0.0,1.0) < pm_) mutate(c2);

            children.push_back({c1, problem->fitness(c1)});
            children.push_back({c2, problem->fitness(c2)});
        }

        evals += popSize_;

        // 2.3) Reemplazo generacional con elitismo
        auto bestParent = *max_element(population.begin(), population.end(),
                                       [](auto& a, auto& b){ return a.fitness < b.fitness; });

        population = move(children);

        auto worstIt = min_element(population.begin(), population.end(),
                                   [](auto& a, auto& b){ return a.fitness < b.fitness; });

        if (bestParent.fitness > worstIt->fitness)
            *worstIt = bestParent;
    }

    // 3) Devolver el mejor
    auto best = *max_element(population.begin(), population.end(),
                             [](auto& a, auto& b){ return a.fitness < b.fitness; });
    return ResultMH(best.sol, best.fitness, evals);
}
