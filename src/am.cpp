#include "am.h"
#include <algorithm>
#include <random>

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
    // 1) Población inicial
    std::vector<Individual> population;
    population.reserve(popSize_);
    for (int i = 0; i < popSize_; ++i) {
        Individual ind;
        ind.sol     = problem->createSolution();
        ind.fitness = problem->fitness(ind.sol);
        population.push_back(ind);
    }
    int evals = popSize_;

    // 2) Búsqueda local (misma instancia)
    LocalSearch ls(lsStrategy_);

    int gen = 0;
    // 3) Bucle generacional
    while (evals < maxEvals) {
        ++gen;
        // 3.1) Selección por torneo k=3
        std::vector<Individual> parents;
        parents.reserve(popSize_);
        for (int i = 0; i < popSize_; ++i)
            parents.push_back(tournamentSelect(population, 3));

        // 3.2) Cruce + mutación => hijos
        std::vector<Individual> children;
        children.reserve(popSize_);
        int n          = problem->getSolutionSize();
        int targetOnes = dynamic_cast<ProblemIncrem*>(problem)->getM();

        for (int i = 0; i < popSize_; i += 2) {
            auto b1 = list2bin(parents[i  ].sol, n);
            auto b2 = list2bin(parents[i+1].sol, n);

            if (Random::get<double>(0.0,1.0) < pc_)
                std::tie(b1,b2) = crossoverUniformBin(b1, b2, targetOnes);

            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b1);
            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b2);

            Individual c1{ bin2list(b1), problem->fitness(bin2list(b1)) };
            Individual c2{ bin2list(b2), problem->fitness(bin2list(b2)) };
            children.push_back(c1);
            children.push_back(c2);
        }
        evals += popSize_;

        // 3.3) Cada 10 generaciones: aplicar BL
        if (gen % 10 == 0) {
            int numLS = std::max(1, int(proportion_ * popSize_));
            if (strategy_ == AMStrategy::All) {
                for (auto &ch : children) {
                    auto res = ls.optimize(problem, 20);
                    ch.sol     = res.solution;
                    ch.fitness = res.fitness;
                    evals     += res.evaluations;
                }
            } else if (strategy_ == AMStrategy::RandomSubset) {
                Random::shuffle(children.begin(), children.end());
                for (int i = 0; i < numLS; ++i) {
                    auto res = ls.optimize(problem, 20);
                    children[i].sol     = res.solution;
                    children[i].fitness = res.fitness;
                    evals               += res.evaluations;
                }
            } else { // BestSubset
                std::nth_element(children.begin(),
                                 children.begin() + numLS,
                                 children.end(),
                                 [](auto &a, auto &b){ return a.fitness < b.fitness; });
                for (int i = 0; i < numLS; ++i) {
                    auto res = ls.optimize(problem, 20);
                    children[i].sol     = res.solution;
                    children[i].fitness = res.fitness;
                    evals               += res.evaluations;
                }
            }
        }

        // 3.4) Reemplazo generacional con elitismo
        auto bestParent = *std::min_element(
            population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; }
        );
        population = std::move(children);
        auto worstIt = std::max_element(
            population.begin(), population.end(),
            [](auto &a, auto &b){ return a.fitness < b.fitness; }
        );
        if (bestParent.fitness < worstIt->fitness)
            *worstIt = bestParent;
    }

    // 4) Devolver mejor final
    auto best = *std::min_element(
        population.begin(), population.end(),
        [](auto &a, auto &b){ return a.fitness < b.fitness; }
    );
    return ResultMH(best.sol, best.fitness, evals);
}
