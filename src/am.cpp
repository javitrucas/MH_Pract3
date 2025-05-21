#include "am.h"
#include <algorithm>

using namespace std;

AM::AM(int popSize,
       double pc,
       double pm,
       double proportion,
       AMStrategy strategy,
       SearchStrategy lsStrategy)
    : popSize_(popSize),
      pc_(pc),
      pm_(pm),
      proportion_(proportion),
      strategy_(strategy),
      crossoverOp_(AMCrossover::CON_ORDEN),
      lsStrategy_(lsStrategy),
      // Fijamos maxLSIters_ según el tipo de BL:
      maxLSIters_((lsStrategy_ == SearchStrategy::LSall) ? 1000
                                                        : 20)
{ }

ResultMH AM::optimize(Problem* problem, int maxEvals) {
    vector<Individual> population;
    population.reserve(popSize_);
    for (int i = 0; i < popSize_; ++i) {
        tSolution sol = problem->createSolution();
        tFitness fit = problem->fitness(sol);
        population.push_back({sol, fit});
    }
    int evals = popSize_;
    int gen = 0;

    // Creamos la BL con la estrategia indicada (LSall o BLsmall)
    LocalSearch ls(lsStrategy_);

    while (evals < maxEvals) {
        ++gen;

        // Selección padres
        vector<Individual> parents;
        parents.reserve(popSize_);
        for (int i = 0; i < popSize_; ++i)
            parents.push_back(tournamentSelect(population, 3));

        // Cruce + mutación
        vector<Individual> children;
        children.reserve(popSize_);
        for (int i = 0; i < popSize_; i += 2) {
            auto &p1 = parents[i];
            auto &p2 = parents[i + 1];

            tSolution c1 = p1.sol, c2 = p2.sol;
            if (Random::get<double>(0.0, 1.0) < pc_) {
                if (crossoverOp_ == AMCrossover::CON_ORDEN)
                    tie(c1, c2) = crossoverConOrden(p1.sol, p2.sol);
                else
                    tie(c1, c2) = crossoverSinOrden(p1.sol, p2.sol);
            }
            if (Random::get<double>(0.0, 1.0) < pm_) mutate(c1);
            if (Random::get<double>(0.0, 1.0) < pm_) mutate(c2);

            children.push_back({c1, problem->fitness(c1)});
            children.push_back({c2, problem->fitness(c2)});
        }
        evals += popSize_;

        // Cada 10 generaciones, aplicamos BL según la estrategia AMStrategy
        if (gen % 10 == 0) {
            int numLS = max(1, int(proportion_ * popSize_));

            auto applyLS = [&](Individual &ind){
                auto res = ls.optimize(problem, maxLSIters_);
                ind.sol     = res.solution;
                ind.fitness = res.fitness;
                evals += res.evaluations;
            };

            if (strategy_ == AMStrategy::All) {
                for (auto &ch : children)
                    applyLS(ch);

            } else if (strategy_ == AMStrategy::RandomSubset) {
                Random::shuffle(children.begin(), children.end());
                for (int i = 0; i < numLS; ++i)
                    applyLS(children[i]);

            } else { // BestSubset
                nth_element(children.begin(),
                            children.begin() + numLS,
                            children.end(),
                            [](auto &a, auto &b){ return a.fitness > b.fitness; });
                for (int i = 0; i < numLS; ++i)
                    applyLS(children[i]);
            }
        }

        // Reemplazo generacional con elitismo
        auto bestParent = *max_element(population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; });

        population = move(children);

        auto worstIt = min_element(population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; });
        if (bestParent.fitness > worstIt->fitness)
            *worstIt = bestParent;
    }

    auto best = *max_element(population.begin(), population.end(),
        [](auto &a, auto &b){ return a.fitness < b.fitness; });
    return ResultMH(best.sol, best.fitness, evals);
}
