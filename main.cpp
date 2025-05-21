#include <iostream>
#include <string>
#include <chrono>
#include "pincrem.h"
#include "randomsearch.h"
#include "greedy.h"
#include "localsearch.h"
#include "agg.h"
#include "age.h"
#include "am.h"
#include <random.hpp>

using namespace std;

void printSolution(const tSolution &sol) {
    for (size_t i = 0; i < sol.size(); ++i)
        cout << sol[i] << (i + 1 < sol.size() ? ", " : "");
}

int main() {
    // Parámetros comunes
    const long seed     = 42;
    const int  m        = 10;
    const double p_icm  = 0.01;
    const int  ev_icm   = 20;

    // Instancias SNAP
    vector<string> instances = {
        "../datos/ca-GrQc.txt",
        "../datos/p2p-Gnutella05.txt",
        "../datos/p2p-Gnutella08.txt",
        "../datos/p2p-Gnutella25.txt"
    };

    // Semilla global
    Random::seed(seed);
    cout << "Batch SNIMP:\n"
         << "  seed=" << seed
         << " | m=" << m
         << " | p=" << p_icm
         << " | ev_icm=" << ev_icm << "\n\n";

    for (auto &inst : instances) {
        cout << "=== Instancia: " << inst << " ===\n";
        ProblemIncrem problem;
        try {
            problem.leerArchivo(inst);
        } catch (exception &e) {
            cerr << "ERROR abriendo " << inst << ": " << e.what() << "\n";
            continue;
        }
        problem.m      = m;
        problem.p_icm  = p_icm;
        problem.ev_icm = ev_icm;

        // 1) RANDOM SEARCH
        {
            cout << "\n-- Random Search --\n";
            RandomSearch rs;
            auto start = chrono::high_resolution_clock::now();
            auto res   = rs.optimize(&problem, 10000);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << chrono::duration<double>(end-start).count()
                 << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }

        // 2) GREEDY
        {
            cout << "\n-- Greedy --\n";
            GreedySearch gs;
            auto start = chrono::high_resolution_clock::now();
            auto res   = gs.optimize(&problem, 0);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << chrono::duration<double>(end-start).count()
                 << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }

        // 3) LOCAL SEARCH (P1a)
        {
            for (auto mode : { SearchStrategy::randLS, SearchStrategy::heurLS }) {
                cout << "\n-- Local Search ("
                     << (mode==SearchStrategy::randLS ? "RandLS" : "HeurLS")
                     << ") --\n";
                LocalSearch ls(mode);
                auto start = chrono::high_resolution_clock::now();
                auto res   = ls.optimize(&problem, 10000);
                auto end   = chrono::high_resolution_clock::now();
                cout << "Best fitness: " << res.fitness
                     << " | evals=" << res.evaluations
                     << " | t(s)=" << chrono::duration<double>(end-start).count()
                     << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
            }
        }

        // 4) AGG (P2)
        {
            for (auto op : { AGGCrossover::UNIFORM, AGGCrossover::POSITION }) {
                cout << "\n-- AGG ("
                     << (op==AGGCrossover::UNIFORM ? "Uniform" : "Position")
                     << ") --\n";
                AGG agg(30, 0.7, 0.1);
                agg.setCrossoverOperator(op);
                auto start = chrono::high_resolution_clock::now();
                auto res   = agg.optimize(&problem, 1000);
                auto end   = chrono::high_resolution_clock::now();
                cout << "Best fitness: " << res.fitness
                     << " | evals=" << res.evaluations
                     << " | t(s)=" << chrono::duration<double>(end-start).count()
                     << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
            }
        }

        // 5) AGE (P2)
        {
            for (auto strat : { CrossoverStrategy::UNIFORM, CrossoverStrategy::POSITION }) {
                cout << "\n-- AGE ("
                     << (strat==CrossoverStrategy::UNIFORM ? "Uniform" : "Position")
                     << ") --\n";
                AGE age(30, 0.1);
                age.setCrossoverStrategy(strat);
                auto start = chrono::high_resolution_clock::now();
                auto res   = age.optimize(&problem, 1000);
                auto end   = chrono::high_resolution_clock::now();
                cout << "Best fitness: " << res.fitness
                     << " | evals=" << res.evaluations
                     << " | t(s)=" << chrono::duration<double>(end-start).count()
                     << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
            }
        }

        // 6) AM (P2)
        {
            vector<string> amNames = { "AM-1 (All)", "AM-2 (RandomSubset)", "AM-3 (BestSubset)" };
            vector<AMStrategy> amStrats = { AMStrategy::All, AMStrategy::RandomSubset, AMStrategy::BestSubset };
            for (int i = 0; i < 3; ++i) {
                cout << "\n-- " << amNames[i] << " + RandLS --\n";
                AM am(30, 0.7, 0.1, 0.1, amStrats[i], SearchStrategy::randLS);
                auto start = chrono::high_resolution_clock::now();
                auto res   = am.optimize(&problem, 1000);
                auto end   = chrono::high_resolution_clock::now();
                cout << "Best fitness: " << res.fitness
                     << " | evals=" << res.evaluations
                     << " | t(s)=" << chrono::duration<double>(end-start).count()
                     << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
            }
        }

        cout << "\n---------------------------------------------\n\n";
    }

    return 0;
}
