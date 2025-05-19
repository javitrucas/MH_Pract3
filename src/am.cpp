#include "am.h"
#include <algorithm>
#include <iostream>

using namespace std;

AM::AM(int popSize, double pc, double pm,
       double proportion, AMStrategy strategy,
       SearchStrategy lsStrategy)
  : popSize_(popSize)
  , pc_(pc)
  , pm_(pm)
  , proportion_(proportion)
  , strategy_(strategy)
  , lsStrategy_(lsStrategy)
{ }

ResultMH AM::optimize(Problem* problem, int maxEvals) {
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

    // 2) Crear instancia única de búsqueda local (aleatoria o heurística)
    LocalSearch ls(lsStrategy_);

    // 3) Bucle generacional
    while (evals < maxEvals) {
        // 3.1) Selección de padres
        vector<Individual> parents;
        parents.reserve(popSize_);
        for (int i = 0; i < popSize_; ++i)
            parents.push_back(tournamentSelect(population, 3));

        // 3.2) Cruce + mutación -> hijos
        vector<Individual> children;
        children.reserve(popSize_);
        int n          = problem->getSolutionSize();
        int targetOnes = dynamic_cast<ProblemIncrem*>(problem)->getM();

        for (int i = 0; i < popSize_; i += 2) {
            auto b1 = list2bin(parents[i  ].sol, n);
            auto b2 = list2bin(parents[i+1].sol, n);

            if (Random::get<double>(0.0,1.0) < pc_)
                tie(b1,b2) = crossoverUniformBin(b1, b2, targetOnes);

            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b1);
            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b2);

            Individual c1{ bin2list(b1), 0 };
            c1.fitness = problem->fitness(c1.sol);
            Individual c2{ bin2list(b2), 0 };
            c2.fitness = problem->fitness(c2.sol);

            children.push_back(c1);
            children.push_back(c2);
        }
        evals += popSize_;

        // 3.3) Búsqueda local sobre hijos según estrategia
        int numLS = max(1, int(proportion_ * popSize_));

        if (strategy_ == AMStrategy::All) {
            for (auto &ch : children) {
                auto res = ls.optimize(problem, 400);
                ch.sol     = res.solution;
                ch.fitness = res.fitness;
                evals      += res.evaluations;
            }
        }
        else if (strategy_ == AMStrategy::RandomSubset) {
            Random::shuffle(children.begin(), children.end());
            for (int i = 0; i < numLS; ++i) {
                auto res = ls.optimize(problem, 400);
                children[i].sol     = res.solution;
                children[i].fitness = res.fitness;
                evals              += res.evaluations;
            }
        }
        else {  // BestSubset
            nth_element(children.begin(),
                        children.begin() + numLS,
                        children.end(),
                        [](auto &a, auto &b){ return a.fitness < b.fitness; });
            for (int i = 0; i < numLS; ++i) {
                auto res = ls.optimize(problem, 400);
                children[i].sol     = res.solution;
                children[i].fitness = res.fitness;
                evals              += res.evaluations;
            }
        }

        // 3.4) Reemplazo generacional con elitismo
        auto bestParent = *min_element(
            population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; }
        );
        population = move(children);
        auto worstIt = max_element(
            population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; }
        );
        if (bestParent.fitness < worstIt->fitness)
            *worstIt = bestParent;
    }

    // 4) Devolver la mejor solución
    auto best = *min_element(
        population.begin(), population.end(),
        [](auto &a, auto &b){ return a.fitness < b.fitness; }
    );
    return ResultMH(best.sol, best.fitness, evals);
}
