#include "am.h"
#include <algorithm>
#include <random>

using namespace std;

AM::AM(int popSize, double pc, double pm,
       double proportion, AMStrategy strategy,
       SearchStrategy lsStrategy)
    : popSize_(popSize),
      pc_(pc),
      pm_(pm),
      proportion_(proportion),
      strategy_(strategy),
      crossoverOp_(AMCrossover::CON_ORDEN),
      lsStrategy_(lsStrategy)
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

    LocalSearch ls(lsStrategy_);
    int gen = 0;

    while (evals < maxEvals) {
        ++gen;

        vector<Individual> parents;
        for (int i = 0; i < popSize_; ++i)
            parents.push_back(tournamentSelect(population, 3));

        vector<Individual> children;
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

        // Aplicar Búsqueda Local cada 10 generaciones
        if (gen % 10 == 0) {
            int numLS = max(1, int(proportion_ * popSize_));
            if (strategy_ == AMStrategy::All) {
                for (auto &ch : children) {
                    auto res = ls.optimize(problem, 20);
                    ch.sol = res.solution;
                    ch.fitness = res.fitness;
                    evals += res.evaluations;
                }
            } else if (strategy_ == AMStrategy::RandomSubset) {
                Random::shuffle(children.begin(), children.end());
                for (int i = 0; i < numLS; ++i) {
                    auto res = ls.optimize(problem, 20);
                    children[i].sol = res.solution;
                    children[i].fitness = res.fitness;
                    evals += res.evaluations;
                }
            } else {
                nth_element(children.begin(), children.begin() + numLS, children.end(),
                            [](const Individual &a, const Individual &b) {
                                return a.fitness > b.fitness;
                            });
                for (int i = 0; i < numLS; ++i) {
                    auto res = ls.optimize(problem, 20);
                    children[i].sol = res.solution;
                    children[i].fitness = res.fitness;
                    evals += res.evaluations;
                }
            }
        }

        // Reemplazo generacional con elitismo
        auto bestParent = *max_element(population.begin(), population.end(),
                                       [](const Individual &a, const Individual &b) {
                                           return a.fitness < b.fitness;
                                       });
        population = move(children);
        auto worstIt = min_element(population.begin(), population.end(),
                                   [](const Individual &a, const Individual &b) {
                                       return a.fitness < b.fitness;
                                   });
        if (bestParent.fitness > worstIt->fitness)
            *worstIt = bestParent;
    }

    auto best = *max_element(population.begin(), population.end(),
                             [](const Individual &a, const Individual &b) {
                                 return a.fitness < b.fitness;
                             });
    return ResultMH(best.sol, best.fitness, evals);
}
