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
#include "es.h"
#include "bmb.h"
#include "ils.h"
#include "grasp.h"
#include <random.hpp>

using namespace std;

void printSolution(const tSolution &sol) {
    for (size_t i = 0; i < sol.size(); ++i)
        cout << sol[i] << (i + 1 < sol.size() ? ", " : "");
}

int main() {
    const long seed     = 42;
    const int  m        = 10;
    const double p_icm  = 0.01;
    const int  ev_icm   = 20;

    vector<string> instances = {
        "../datos/ca-GrQc.txt",
        "../datos/p2p-Gnutella05.txt",
        "../datos/p2p-Gnutella08.txt",
        "../datos/p2p-Gnutella25.txt"
    };

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
            for (auto mode : { SearchStrategy::LSall, SearchStrategy::BLsmall }) {
                cout << "\n-- Local Search ("
                     << (mode == SearchStrategy::LSall ? "LSall" : "BLsmall")
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
            for (auto op : { AGGCrossover::CON_ORDEN, AGGCrossover::SIN_ORDEN }) {
                cout << "\n-- AGG ("
                     << (op == AGGCrossover::CON_ORDEN ? "Con Orden" : "Sin Orden")
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
            for (auto strat : { CrossoverStrategy::CON_ORDEN, CrossoverStrategy::SIN_ORDEN }) {
                cout << "\n-- AGE ("
                     << (strat == CrossoverStrategy::CON_ORDEN ? "Con Orden" : "Sin Orden")
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
                // Primer memético usa LSall, los otros dos usan BLsmall
                SearchStrategy lsMode = (i == 0 ? SearchStrategy::LSall : SearchStrategy::BLsmall);
                cout << "\n-- " << amNames[i]
                     << " + " << (lsMode == SearchStrategy::LSall ? "LSall" : "BLsmall")
                     << " --\n";
                AM am(30, 0.7, 0.1, 0.1, amStrats[i], lsMode);
                auto start = chrono::high_resolution_clock::now();
                auto res   = am.optimize(&problem, 1000);
                auto end   = chrono::high_resolution_clock::now();
                cout << "Best fitness: " << res.fitness
                     << " | evals=" << res.evaluations
                     << " | t(s)=" << chrono::duration<double>(end-start).count()
                     << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
            }
        }

        // 7) ES (P3)
        {
            cout << "\n-- Enfriamiento Simulado --\n";
            ES es;
            auto start = chrono::high_resolution_clock::now();
            auto res = es.optimize(&problem, 1000);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << chrono::duration<double>(end-start).count()
                 << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }

        // 8) BMB (P3)
        {
            cout << "\n-- BMB --\n";
            BMB bmb;
            auto start = chrono::high_resolution_clock::now();
            auto res = bmb.optimize(&problem, 0);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << chrono::duration<double>(end-start).count()
                 << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }

        // 9) ILS (P3)
        {
            cout << "\n-- ILS --\n";
            ILS ils;
            auto start = chrono::high_resolution_clock::now();
            auto res = ils.optimize(&problem, 0);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << chrono::duration<double>(end-start).count()
                 << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }


        // 10) GRASP-NOBL (P3)
        {
            cout << "\n-- GRASP-NOBL --\n";
            GRASP grasp(GRASP::Mode::NOBL);
            auto start = chrono::high_resolution_clock::now();
            auto res = grasp.optimize(&problem, 0);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                << " | evals=" << res.evaluations
                << " | t(s)=" << chrono::duration<double>(end-start).count()
                << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }

        // 11) GRASP-SIBL (P3)
        {
            cout << "\n-- GRASP-SIBL --\n";
            GRASP grasp(GRASP::Mode::SIBL);
            auto start = chrono::high_resolution_clock::now();
            auto res = grasp.optimize(&problem, 0);
            auto end   = chrono::high_resolution_clock::now();
            cout << "Best fitness: " << res.fitness
                << " | evals=" << res.evaluations
                << " | t(s)=" << chrono::duration<double>(end-start).count()
                << "\nSolution: ["; printSolution(res.solution); cout << "]\n";
        }


        cout << "\n---------------------------------------------\n\n";
    }

    return 0;
}
