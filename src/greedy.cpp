#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <random>
#include "greedy.h"
#include "pincrem.h"

using namespace std;

template<typename T>
void logVector(const string& name, const vector<T>& v) {
    cout << name << ": ";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << v[i] << (i + 1 < v.size() ? ", " : "\n");
    }
    if (v.empty()) cout << "(vacío)\n";
}

// Encuentra el mejor candidato disponible según fitness voraz
tDomain selectNext(Problem* problem,
                   const vector<tDomain>& solution,
                   const vector<bool>& available) {
    tFitness bestFit = numeric_limits<tFitness>::max();
    tDomain bestIdx = -1;
    vector<tDomain> temp;
    temp.reserve(solution.size() + 1);

    for (size_t idx = 0; idx < available.size(); ++idx) {
        if (!available[idx]) continue;
        temp = solution;
        temp.push_back(static_cast<tDomain>(idx));
        tFitness fit = problem->fitness(temp);
        if (fit < bestFit) {
            bestFit = fit;
            bestIdx = static_cast<tDomain>(idx);
        }
    }
    return bestIdx;
}

ResultMH GreedySearch::optimize(Problem* problem, int maxEvals) {
    assert(maxEvals > 0);
    auto* inc = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = inc->getSolutionSize();
    int m = inc->getM();

    vector<tDomain> solution;
    solution.reserve(m);
    vector<bool> available(n, true);

    mt19937 rng(Random::get<int>(0, 1<<30));
    uniform_int_distribution<int> dist(0, static_cast<int>(n) - 1);

    // Selección del primer candidato aleatorio
    tDomain first = static_cast<tDomain>(dist(rng));
    solution.push_back(first);
    available[first] = false;
    cout << "Primer candidato seleccionado: " << first << endl;

    // Imprimir candidatos restantes
    vector<tDomain> remaining;
    remaining.reserve(n - 1);
    for (size_t i = 0; i < n; ++i) 
        if (available[i]) remaining.push_back(static_cast<tDomain>(i));
    logVector("Candidatos restantes", remaining);

    // Selección voraz de los siguientes m-1 elementos
    for (int step = 1; step < m; ++step) {
        tDomain pick = selectNext(problem, solution, available);
        if (pick < 0) break;
        solution.push_back(pick);
        available[pick] = false;

        cout << "Candidato añadido: " << pick << endl;
        logVector("Nueva solución", solution);

        remaining.clear();
        for (size_t i = 0; i < n; ++i) 
            if (available[i]) remaining.push_back(static_cast<tDomain>(i));
        logVector("Candidatos restantes", remaining);
    }

    // Evaluación final
    tFitness finalFit = problem->fitness(solution);
    return { solution, finalFit, 1 };
}
