#include "localsearch.h"
#include "pincrem.h"
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <random>
#include <cassert>

using namespace std;

ResultMH LocalSearch::optimize(Problem* problem, int maxevals) {
    assert(maxevals > 0);

    // Datos del problema
    auto* realP = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = problem->getSolutionSize();        // universo
    tSolution sol = problem->createSolution();    // solución inicial
    size_t m = sol.size();                        // tamaño de la solución

    // Evaluación inicial y data para factorización
    tFitness fit = problem->fitness(sol);
    auto* info = problem->generateFactoringInfo(sol);
    int evals = 1;

    // posOf[x] = posición de x en sol, o -1 si no está
    vector<int> posOf(n, -1);
    for (int i = 0; i < (int)m; ++i)
        posOf[ sol[i] ] = i;

    // Conjuntos de seleccionados / no seleccionados
    unordered_set<int> sel(sol.begin(), sol.end()), nonSel;
    nonSel.reserve(n - m);
    for (int i = 0; i < (int)n; ++i)
        if (!sel.count(i))
            nonSel.insert(i);

    // Vecindario reutilizable
    vector<pair<int,int>> neigh;
    neigh.reserve(m * nonSel.size());

    // RNG para randLS
    mt19937 rng(random_device{}());

    bool improved = true;
    while (improved && evals < maxevals) {
        improved = false;
        neigh.clear();

        // Generar todos los swaps posibles (u ∈ sel, v ∈ nonSel)
        for (int u : sel)
            for (int v : nonSel)
                neigh.emplace_back(u, v);

        if (neigh.empty()) break;

        if (explorationMode == SearchStrategy::heurLS) {
            // Best-improvement (mejor mejora)
            tFitness bestFit = fit;
            int bestU = -1, bestV = -1, bestPos = -1;

            for (auto [u, v] : neigh) {
                if (evals >= maxevals) break;
                int pos = posOf[u];
                assert(pos >= 0 && pos < (int)sol.size());
                tFitness cand = problem->fitness(sol, info, pos, v);
                ++evals;
                if (cand < bestFit) {
                    bestFit = cand;
                    bestU = u; bestV = v; bestPos = pos;
                }
            }

            if (bestU != -1) {
                // Aplicar swap óptimo
                sel.erase(bestU); sel.insert(bestV);
                nonSel.erase(bestV); nonSel.insert(bestU);

                sol[bestPos] = bestV;
                posOf[bestV] = bestPos;
                posOf[bestU] = -1;

                problem->updateSolutionFactoringInfo(info, sol, bestPos, bestV);
                fit = bestFit;
                improved = true;
            }
        }
        else {
            // randLS: primer mejor en orden aleatorio
            shuffle(neigh.begin(), neigh.end(), rng);
            for (auto [u, v] : neigh) {
                if (evals >= maxevals) break;
                int pos = posOf[u];
                assert(pos >= 0 && pos < (int)sol.size());
                tFitness cand = problem->fitness(sol, info, pos, v);
                ++evals;
                if (cand < fit) {
                    // Aplicar swap inmediato
                    sel.erase(u); sel.insert(v);
                    nonSel.erase(v); nonSel.insert(u);

                    sol[pos]     = v;
                    posOf[v]     = pos;
                    posOf[u]     = -1;

                    problem->updateSolutionFactoringInfo(info, sol, pos, v);
                    fit = cand;
                    improved = true;
                    break;  // salimos al primer mejora
                }
            }
        }
    }

    delete info;
    return ResultMH(sol, fit, evals);
}
