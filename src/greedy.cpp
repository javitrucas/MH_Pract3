#include <iostream>
#include <vector>
#include <algorithm>
#include "greedy.h"
#include "pincrem.h"

using namespace std;

ResultMH GreedySearch::optimize(Problem* problem, int /*maxEvals*/) {
    auto* inc = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = inc->getSolutionSize();
    int m = inc->getM();

    vector<bool> selected(n, false);
    tSolution S;

    cout << "[Greedy] Construyendo solución voraz (m=" << m << ")..." << endl;

    for (int i = 0; i < m; ++i) {
        int best_node = -1;
        double best_score = numeric_limits<double>::lowest();

        for (int u = 0; u < (int)n; ++u) {
            if (selected[u]) continue;

            int d_u = inc->adj[u].size();
            int sum_deg = 0;
            for (int v : inc->adj[u]) sum_deg += inc->adj[v].size();
            double score = d_u + sum_deg;

            if (score > best_score) {
                best_score = score;
                best_node = u;
            }
        }

        if (best_node >= 0) {
            selected[best_node] = true;
            S.push_back(best_node);
            cout << "  Añadido nodo " << best_node << " con heurística = " << best_score << endl;
        }
    }

    tFitness fit = problem->fitness(S);
    return ResultMH(S, fit, S.size());
}