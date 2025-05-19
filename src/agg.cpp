#include "agg.h"
#include <algorithm>
#include <iostream>

using namespace std;

AGG::AGG(int popSize, double pc, double pm)
  : popSize_(popSize)
  , pc_(pc)
  , pm_(pm)
  , crossoverOp_(AGGCrossover::UNIFORM)
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
        // 2.1) Selección de padres (torneo k=3, popSize_ veces)
        vector<Individual> parents;
        parents.reserve(popSize_);
        for (int i = 0; i < popSize_; ++i) {
            parents.push_back(tournamentSelect(population, 3));
        }

        // 2.2) Recombinar + mutar -> hijos
        vector<Individual> children;
        children.reserve(popSize_);
        int n = problem->getSolutionSize();
        int targetOnes = dynamic_cast<ProblemIncrem*>(problem)->getM();

        for (int i = 0; i < popSize_; i += 2) {
            // Emparejar padres i, i+1
            auto &p1 = parents[i];
            auto &p2 = parents[i+1];

            // convertir a binario
            auto b1 = list2bin(p1.sol, n);
            auto b2 = list2bin(p2.sol, n);

            // cruce con probabilidad pc_
            if (Random::get<double>(0.0,1.0) < pc_) {
                if (crossoverOp_ == AGGCrossover::UNIFORM) {
                    tie(b1, b2) = crossoverUniformBin(b1, b2, targetOnes);
                } else {
                    tie(b1, b2) = crossoverPositionBin(b1, b2);
                }
            }

            // mutación por individuo
            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b1);
            if (Random::get<double>(0.0,1.0) < pm_) mutateBin(b2);

            // reconstruir lista y evaluar
            Individual c1, c2;
            c1.sol     = bin2list(b1);
            c1.fitness = problem->fitness(c1.sol);
            c2.sol     = bin2list(b2);
            c2.fitness = problem->fitness(c2.sol);
            children.push_back(c1);
            children.push_back(c2);
        }
        evals += popSize_;

        // 2.3) Reemplazo generacional con elitismo
        //  - conservar mejor de padres si no está entre hijos
        //  encontrar mejor padre
        auto bestParent = *min_element(population.begin(), population.end(),
                                       [](auto &a, auto &b){
                                           return a.fitness < b.fitness;
                                       });
        // reemplazar población completa por hijos
        population = move(children);
        // encontrar peor hijo
        auto worstIt = max_element(population.begin(), population.end(),
                                   [](auto &a, auto &b){
                                       return a.fitness < b.fitness;
                                   });
        // insertar elitismo
        if (bestParent.fitness < worstIt->fitness) {
            *worstIt = bestParent;
        }
    }

    // 3) Devolver mejor final
    auto best = *min_element(population.begin(), population.end(),
                             [](auto &a, auto &b){ return a.fitness < b.fitness; });
    return ResultMH(best.sol, best.fitness, evals);
}
