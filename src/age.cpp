#include "age.h"
#include <algorithm>
#include <iostream>

using namespace std;

AGE::AGE(int popSize, double pm)
  : popSize_(popSize)
  , pm_(pm)
  , crossoverStrategy_(CrossoverStrategy::CON_ORDEN)  // valor por defecto
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

        // Cruce entero
        tSolution c1, c2;
        if (crossoverStrategy_ == CrossoverStrategy::CON_ORDEN) {
            tie(c1, c2) = crossoverConOrden(p1.sol, p2.sol);
        } else {
            tie(c1, c2) = crossoverSinOrden(p1.sol, p2.sol);
        }

        // Mutación
        if (Random::get<double>(0.0,1.0) < pm_) mutate(c1);
        if (Random::get<double>(0.0,1.0) < pm_) mutate(c2);

        // Evaluar hijos
        Individual ind1{c1, problem->fitness(c1)};
        Individual ind2{c2, problem->fitness(c2)};
        evals += 2;

        // Reemplazo estacionario: sustituyen a los 2 peores
        sort(population.begin(), population.end(),
             [](auto &a, auto &b){ return a.fitness < b.fitness; }); // ascendente

        if (ind1.fitness > population[0].fitness)
            population[0] = ind1;
        if (ind2.fitness > population[1].fitness)
            population[1] = ind2;
    }

    // 3) Devolver el mejor
    auto best = *max_element(population.begin(), population.end(),
                             [](auto &a, auto &b){ return a.fitness < b.fitness; });
    return ResultMH(best.sol, best.fitness, evals);
}
