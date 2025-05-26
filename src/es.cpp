#include "es.h"
#include <cmath>
#include <random.hpp>
#include <algorithm>
#include <cassert>

using namespace std;

ResultMH ES::optimize(Problem* problem, int maxEvals) {
    const double phi = 0.3;
    const double mu = 0.2;
    const double Tf = 1e-3;

    auto* realP = dynamic_cast<ProblemIncrem*>(problem);
    size_t n = problem->getSolutionSize();
    tSolution current = problem->createSolution();
    size_t m = current.size();

    tFitness fCurrent = problem->fitness(current);
    tSolution best = current;
    tFitness fBest = fCurrent;

    int evals = 1;
    int maxVecinos = 5 * m;
    int maxExitos = max(1, maxVecinos / 10);
    int M = maxEvals / maxVecinos;

    double T0 = (mu * fCurrent) / (-log(phi));
    double beta = (T0 - Tf) / (M * T0 * Tf);
    double T = T0;

    unordered_set<int> sel(current.begin(), current.end()), nonSel;
    for (int i = 0; i < (int)n; ++i)
        if (!sel.count(i)) nonSel.insert(i);

    vector<int> selVec(current.begin(), current.end());

    while (evals < maxEvals && T > Tf) {
        int exitos = 0, vecinos = 0;
        vector<bool> usado(m, false);

        while (vecinos < maxVecinos && exitos < maxExitos && evals < maxEvals) {
            int pos;
            do {
                pos = Random::get<int>(0, m - 1);
            } while (usado[pos]);
            usado[pos] = true;

            int u = selVec[pos];
            int v = *next(nonSel.begin(), Random::get<int>(0, nonSel.size() - 1));

            tSolution neighbor = current;
            neighbor[pos] = v;
            tFitness fNeighbor = problem->fitness(neighbor);
            ++evals;
            ++vecinos;

            double delta = fCurrent - fNeighbor;
            bool accept = (delta < 0) || (Random::get<double>(0.0, 1.0) < exp(-delta / T));

            if (accept) {
                current = neighbor;
                fCurrent = fNeighbor;
                sel.erase(u); sel.insert(v);
                nonSel.erase(v); nonSel.insert(u);
                selVec[pos] = v;
                ++exitos;

                if (fCurrent > fBest) {
                    best = current;
                    fBest = fCurrent;
                }
            }
        }

        if (exitos == 0) break;
        T = T / (1 + beta * T);
    }

    return ResultMH(best, fBest, evals);
}
