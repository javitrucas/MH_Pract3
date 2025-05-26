// src/grasp.cpp

#include "grasp.h"
#include <random.hpp>
#include <algorithm>
#include <limits>

using namespace std;

ResultMH GRASP::optimize(Problem* problem, int /*maxEvals*/) {
    constexpr int runs = 10;
    const int blMaxEvals = 100;

    auto* realP = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = problem->getSolutionSize();
    size_t m = problem->createSolution().size();

    tSolution bestSol;
    tFitness bestFit = numeric_limits<tFitness>::lowest();
    int totalEvals = 0;

    for (int run = 0; run < runs; ++run) {
        // Construcción GRASP aleatorizada
        vector<int> sel;
        sel.reserve(m);
        vector<bool> inSel(n, false);

        // Paso 1: primera selección aleatoria
        int first = Random::get<int>(0, (int)n - 1);
        sel.push_back(first);
        inSel[first] = true;

        // Resto de m-1 pasos
        for (size_t step = 1; step < m; ++step) {
            tFitness hmax = numeric_limits<tFitness>::lowest();
            tFitness hmin = numeric_limits<tFitness>::max();
            vector<pair<int,tFitness>> hcands;
            hcands.reserve(n - step);

            for (int u = 0; u < (int)n; ++u) {
                if (inSel[u]) continue;
                tFitness h = computeHeuristic(realP, u);
                hcands.emplace_back(u, h);
                hmax = max(hmax, h);
                hmin = min(hmin, h);
            }
            double r = Random::get<double>(0.0, 1.0);
            double thr = hmax - r * (hmax - hmin);

            vector<int> lrc;
            for (auto &p : hcands)
                if (p.second >= thr)
                    lrc.push_back(p.first);

            int idx = Random::get<int>(0, (int)lrc.size() - 1);
            int chosen = lrc[idx];
            sel.push_back(chosen);
            inSel[chosen] = true;
        }

        // Convertir a tSolution
        tSolution sol(m);
        for (size_t i = 0; i < m; ++i) sol[i] = sel[i];

        // Evaluar o aplicar BL
        if (mode_ == Mode::NOBL) {
            tFitness fit = problem->fitness(sol);
            totalEvals += 1;
            if (fit > bestFit) {
                bestFit = fit;
                bestSol = sol;
            }
        } else {
            LocalSearch ls(SearchStrategy::BLsmall);
            ResultMH res = ls.optimize(problem, blMaxEvals);
            totalEvals += res.evaluations;
            if (res.fitness > bestFit) {
                bestFit = res.fitness;
                bestSol = res.solution;
            }
        }
    }

    return ResultMH(bestSol, bestFit, totalEvals);
}

// Heurística basada en grado y grado de vecinos (gne)
tFitness GRASP::computeHeuristic(ProblemIncrem* realP, int u) {
    // Usar adj directamente
    const auto &neighbors = realP->adj[u];
    tFitness sum = neighbors.size();
    for (int v : neighbors) {
        sum += realP->adj[v].size();
    }
    return sum;
}
