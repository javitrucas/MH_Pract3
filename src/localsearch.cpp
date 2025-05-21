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

    vector<int> posOf(n, -1);
    for (int i = 0; i < (int)m; ++i)
        posOf[sol[i]] = i;

    unordered_set<int> sel(sol.begin(), sol.end()), nonSel;
    for (int i = 0; i < (int)n; ++i)
        if (!sel.count(i)) nonSel.insert(i);

    vector<pair<int,int>> neigh;
    mt19937 rng(Random::get<int>(0, 1<<30));

    bool improved = true;
    while (improved && evals < maxevals) {
        improved = false;
        neigh.clear();

        for (int u : sel)
            for (int v : nonSel)
                neigh.emplace_back(u, v);
        if (neigh.empty()) break;

        if (explorationMode == SearchStrategy::heurLS) {
            tFitness bestFit = numeric_limits<tFitness>::lowest();
            int bestU=-1, bestV=-1, bestPos=-1;

            for (auto [u,v] : neigh) {
                if (evals >= maxevals) break;
                int pos = posOf[u]; assert(pos>=0);
                sol[pos] = v;
                tFitness cand = problem->fitness(sol);
                ++evals;
                sol[pos] = u;
                if (cand > bestFit) {
                    bestFit=cand; bestU=u; bestV=v; bestPos=pos;
                }
            }
            if (bestU!=-1) {
                sel.erase(bestU); sel.insert(bestV);
                nonSel.erase(bestV); nonSel.insert(bestU);
                sol[bestPos] = bestV;
                posOf[bestV] = bestPos; posOf[bestU] = -1;
                fit = bestFit;
                improved = true;
            }
        } else {
            shuffle(neigh.begin(), neigh.end(), rng);
            for (auto [u,v] : neigh) {
                if (evals >= maxevals) break;
                int pos = posOf[u]; assert(pos>=0);
                sol[pos] = v;
                tFitness cand = problem->fitness(sol);
                ++evals;
                if (cand > fit) {
                    sel.erase(u); sel.insert(v);
                    nonSel.erase(v); nonSel.insert(u);
                    fit = cand;
                    sol[pos] = v;
                    posOf[v] = pos; posOf[u] = -1;
                    improved = true;
                    break;
                }
                sol[pos] = u;
            }
        }
    }

    return ResultMH(sol, fit, evals);
}