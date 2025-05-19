#include "genetic_utils.h"
#include "random.hpp"
#include <numeric>
#include <algorithm>

std::vector<int> GeneticUtils::list2bin(const tSolution& lst, int n) const {
    std::vector<int> bin(n, 0);
    for (int idx : lst)
        if (idx >= 0 && idx < n)
            bin[idx] = 1;
    return bin;
}

tSolution GeneticUtils::bin2list(const std::vector<int>& bin) const {
    tSolution lst;
    for (int i = 0; i < (int)bin.size(); ++i)
        if (bin[i] == 1)
            lst.push_back(i);
    return lst;
}

Individual GeneticUtils::tournamentSelect(const std::vector<Individual>& pop, int k) const {
    int idx = Random::get(0, (int)pop.size() - 1);
    Individual best = pop[idx];
    for (int i = 1; i < k; ++i) {
        idx = Random::get(0, (int)pop.size() - 1);
        if (pop[idx].fitness < best.fitness)
            best = pop[idx];
    }
    return best;
}

std::pair<std::vector<int>, std::vector<int>>
GeneticUtils::crossoverUniformBin(const std::vector<int>& b1,
                                  const std::vector<int>& b2,
                                  int targetOnes) const {
    int n = (int)b1.size();
    std::vector<int> c1(n, 0), c2(n, 0);
    for (int i = 0; i < n; ++i)
        if (b1[i] == b2[i])
            c1[i] = c2[i] = b1[i];

    std::vector<int> idxs;
    for (int i = 0; i < n; ++i)
        if (b1[i] != b2[i])
            idxs.push_back(i);

    Random::shuffle(idxs.begin(), idxs.end());
    int half = (int)idxs.size() / 2;
    for (int i = 0; i < (int)idxs.size(); ++i) {
        int pos = idxs[i];
        if (i < half) {
            c1[pos] = b1[pos];
            c2[pos] = b2[pos];
        } else {
            c1[pos] = b2[pos];
            c2[pos] = b1[pos];
        }
    }

    auto repair = [&](std::vector<int>& sol) {
        int cnt = std::accumulate(sol.begin(), sol.end(), 0);
        if (cnt > targetOnes) {
            std::vector<int> ones;
            for (int i = 0; i < n; ++i)
                if (sol[i] == 1) ones.push_back(i);
            Random::shuffle(ones.begin(), ones.end());
            for (int j = 0; j < cnt - targetOnes; ++j)
                sol[ones[j]] = 0;
        } else if (cnt < targetOnes) {
            std::vector<int> zeros;
            for (int i = 0; i < n; ++i)
                if (sol[i] == 0) zeros.push_back(i);
            Random::shuffle(zeros.begin(), zeros.end());
            for (int j = 0; j < targetOnes - cnt; ++j)
                sol[zeros[j]] = 1;
        }
    };

    repair(c1);
    repair(c2);
    return {c1, c2};
}

std::pair<std::vector<int>, std::vector<int>>
GeneticUtils::crossoverPositionBin(const std::vector<int>& b1,
                                   const std::vector<int>& b2) const {
    int n = (int)b1.size();
    std::vector<int> c1 = b1, c2 = b2;
    std::vector<int> ones1, ones2;

    for (int i = 0; i < n; ++i) {
        if (b1[i] == 1 && b2[i] == 0) ones1.push_back(i);
        if (b2[i] == 1 && b1[i] == 0) ones2.push_back(i);
    }

    Random::shuffle(ones1.begin(), ones1.end());
    Random::shuffle(ones2.begin(), ones2.end());

    int k = std::min(ones1.size(), ones2.size());
    for (int i = 0; i < k; ++i) {
        int p1 = ones1[i], p2 = ones2[i];
        c1[p1] = 0; c1[p2] = 1;
        c2[p1] = 1; c2[p2] = 0;
    }

    return {c1, c2};
}

void GeneticUtils::mutateBin(std::vector<int>& bin) const {
    int n = (int)bin.size();
    std::vector<int> ones, zeros;
    for (int i = 0; i < n; ++i) {
        if (bin[i] == 1) ones.push_back(i);
        else             zeros.push_back(i);
    }

    if (ones.empty() || zeros.empty()) return;

    int i1 = Random::get(0, (int)ones.size() - 1);
    int i0 = Random::get(0, (int)zeros.size() - 1);
    bin[ones[i1]] = 0;
    bin[zeros[i0]] = 1;
}
