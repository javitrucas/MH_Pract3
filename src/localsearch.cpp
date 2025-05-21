#include "localsearch.h"
#include "pincrem.h"
#include <algorithm>
#include <unordered_set>
#include <cassert>

using namespace std;

ResultMH LocalSearch::optimize(Problem* problem, int maxevals) {
    assert(maxevals > 0);

    auto* realP = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = problem->getSolutionSize();
    tSolution sol = problem->createSolution();
    size_t m = sol.size();

    tFitness fit = problem->fitness(sol);
    int evals = 1;
    int evalsWithoutImprovement = 0;

    vector<int> posOf(n, -1);
    for (int i = 0; i < (int)m; ++i)
        posOf[sol[i]] = i;

    unordered_set<int> sel(sol.begin(), sol.end()), nonSel;
    for (int i = 0; i < (int)n; ++i)
        if (!sel.count(i)) nonSel.insert(i);

    vector<pair<int,int>> neigh;

    bool improved = true;
    while (improved && evals < maxevals) {
        improved = false;
        neigh.clear();

        // Generar todos los movimientos de intercambio (u en sel, v en nonSel)
        for (int u : sel)
            for (int v : nonSel)
                neigh.emplace_back(u, v);
        if (neigh.empty()) break;

        // Barajado Fisher–Yates usando Random::get
        for (int i = (int)neigh.size() - 1; i > 0; --i) {
            int j = Random::get<int>(0, i);
            std::swap(neigh[i], neigh[j]);
        }

        // Primer‐mejor
        for (auto [u, v] : neigh) {
            if (evals >= maxevals) break;

            int pos = posOf[u];
            assert(pos >= 0);
            sol[pos] = v;
            tFitness cand = problem->fitness(sol);
            ++evals;

            if (cand > fit) {
                // Aceptar mejora
                sel.erase(u); sel.insert(v);
                nonSel.erase(v); nonSel.insert(u);
                fit = cand;
                posOf[v] = pos; posOf[u] = -1;
                improved = true;
                evalsWithoutImprovement = 0;
                break;
            } else {
                // Revertir y contar sin mejora para BLsmall
                sol[pos] = u;
                if (explorationMode == SearchStrategy::BLsmall)
                    ++evalsWithoutImprovement;
            }
        }

        // Criterio de parada adicional para BLsmall
        if (explorationMode == SearchStrategy::BLsmall && evalsWithoutImprovement >= 20)
            break;
    }

    return ResultMH(sol, fit, evals);
}
