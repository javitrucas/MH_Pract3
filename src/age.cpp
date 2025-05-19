#include "age.h"
#include <algorithm>
#include <iostream>

using namespace std;

AGE::AGE(int popSize, double pm)
  : popSize_(popSize)
  , pm_(pm)
  , crossoverStrategy_(CrossoverStrategy::UNIFORM)
{ }

ResultMH AGE::optimize(Problem* problem, int maxEvals) {
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

    // 2) Bucle principal
    while (evals < maxEvals) {
        // Selección de padres
        Individual p1 = tournamentSelect(population, 3);
        Individual p2 = tournamentSelect(population, 3);

        // Convertir a binario
        int n = problem->getSolutionSize();
        auto b1 = list2bin(p1.sol, n);
        auto b2 = list2bin(p2.sol, n);

        // Cruce
        vector<int> cb1, cb2;
        int targetOnes = dynamic_cast<ProblemIncrem*>(problem)->getM();
        if (crossoverStrategy_ == CrossoverStrategy::UNIFORM) {
            tie(cb1, cb2) = crossoverUniformBin(b1, b2, targetOnes);
        } else {
            tie(cb1, cb2) = crossoverPositionBin(b1, b2);
        }

        // Mutación
        if (Random::get<double>(0.0,1.0) < pm_) mutateBin(cb1);
        if (Random::get<double>(0.0,1.0) < pm_) mutateBin(cb2);

        // Reconstruir y evaluar hijos
        Individual cind1, cind2;
        cind1.sol     = bin2list(cb1);
        cind1.fitness = problem->fitness(cind1.sol);
        cind2.sol     = bin2list(cb2);
        cind2.fitness = problem->fitness(cind2.sol);
        evals += 2;

        // Reemplazo estacionario (2 hijos compiten c/ 2 peores)
        sort(population.begin(), population.end(),
             [](auto &a, auto &b){ return a.fitness > b.fitness; }); // descendente
        if (cind1.fitness < population[0].fitness)
            population[0] = cind1;
        if (cind2.fitness < population[1].fitness)
            population[1] = cind2;
    }

    // 3) Devolver mejor
    auto best = *min_element(population.begin(), population.end(),
                             [](auto &a, auto &b){ return a.fitness < b.fitness; });
    return ResultMH(best.sol, best.fitness, evals);
}
