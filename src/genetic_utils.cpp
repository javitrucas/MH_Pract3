#include "genetic_utils.h"
#include "random.hpp"
#include <numeric>
#include <algorithm>

Individual GeneticUtils::tournamentSelect(const std::vector<Individual>& pop, int k) const {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)pop.size() - 1);
    Individual best = pop[dist(rng)];
    for (int i = 1; i < k; ++i) {
        const Individual &cand = pop[dist(rng)];
        if (cand.fitness > best.fitness) best = cand; // Maximizar
    }
    return best;
}

std::pair<tSolution, tSolution> GeneticUtils::crossoverSinOrden(const tSolution& p1,
                                                                const tSolution& p2) const {
    int m = p1.size();
    std::uniform_int_distribution<int> dist(0, m - 1);
    static thread_local std::mt19937 rng(std::random_device{}());

    int cut1 = dist(rng);
    int cut2 = dist(rng);
    if (cut1 > cut2) std::swap(cut1, cut2);

    tSolution c1 = p1, c2 = p2;

    std::vector<int> taken1, taken2;

    for (int i = cut1; i <= cut2; ++i) {
        taken1.push_back(p1[i]);
        taken2.push_back(p2[i]);
    }

    auto repair = [&](tSolution& child, const std::vector<int>& taken, const tSolution& other) {
        int idx = 0;
        for (int i = 0; i < m; ++i) {
            if (i >= cut1 && i <= cut2) continue;
            while (std::find(taken.begin(), taken.end(), other[idx]) != taken.end()) {
                idx++;
            }
            child[i] = other[idx++];
        }
    };

    repair(c1, taken1, p2);
    repair(c2, taken2, p1);

    return {c1, c2};
}

std::pair<tSolution, tSolution> GeneticUtils::crossoverConOrden(const tSolution& p1,
                                                                const tSolution& p2) const {
    int m = p1.size();
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, m - 1);

    // Elegimos puntos de corte
    int cut1 = dist(rng), cut2 = dist(rng);
    if (cut1 > cut2) std::swap(cut1, cut2);

    tSolution c1(m, -1), c2(m, -1);

    // Copiar segmento central
    for (int i = cut1; i <= cut2; ++i) {
        c1[i] = p1[i];
        c2[i] = p2[i];
    }

    auto fill = [&](const tSolution& donor, tSolution& child) {
        int idx = (cut2 + 1) % m;
        for (int i = 0; i < m; ++i) {
            int val = donor[(cut2 + 1 + i) % m];
            if (std::find(child.begin(), child.end(), val) == child.end()) {
                child[idx] = val;
                idx = (idx + 1) % m;
            }
        }
    };

    fill(p2, c1);
    fill(p1, c2);

    return {c1, c2};
}

void GeneticUtils::mutate(std::vector<int>& sol) const {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, sol.size() - 1);
    int i = dist(rng), j = dist(rng);
    std::swap(sol[i], sol[j]);
}
