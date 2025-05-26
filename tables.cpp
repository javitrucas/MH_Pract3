// tables.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <filesystem>
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
#include "ils_es.h"
#include <random.hpp>

using namespace std;
namespace fs = std::filesystem;

void printSolution(const tSolution &sol) {
    for (size_t i = 0; i < sol.size(); ++i)
        cout << sol[i] << (i + 1 < sol.size() ? ", " : "");
}

int main() {
    const long seed     = 42;
    const int  m        = 10;
    const double p_icm  = 0.01;
    const int  ev_icm   = 20;

    // Crear carpeta output si no existe
    fs::create_directories("../output");

    // Salidas
    ofstream csv("../output/results.csv");
    csv << "Instancia,Algoritmo,Fitness,Evals,Time\n";
    ofstream txt("../output/results.txt");

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
        txt << "=== Instancia: " << inst << " ===\n";

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

        auto run_and_record = [&](const string &name, MH &alg, int maxEvals=0) {
            auto start = chrono::high_resolution_clock::now();
            auto res   = alg.optimize(&problem, maxEvals);
            auto end   = chrono::high_resolution_clock::now();
            double t   = chrono::duration<double>(end - start).count();

            // Pantalla
            cout << "-- " << name << " --\n"
                 << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << fixed << setprecision(4) << t << "\n"
                 << "Solution: ["; printSolution(res.solution); cout << "]\n\n";

            // TXT
            txt << "-- " << name << " --\n"
                << "Best fitness: " << res.fitness
                << " | evals=" << res.evaluations
                << " | t(s)=" << fixed << setprecision(4) << t << "\n"
                << "Solution: [";
            for (size_t i = 0; i < res.solution.size(); ++i)
                txt << res.solution[i] << (i + 1 < res.solution.size() ? ", " : "");
            txt << "]\n\n";

            // CSV
            csv << inst << "," << name << ","
                << res.fitness << ","
                << res.evaluations << ","
                << t << "\n";

            return res.fitness;
        };

        // 1) RANDOM SEARCH
        { RandomSearch rs; run_and_record("RandomSearch", rs, 10000); }

        // 2) GREEDY
        { GreedySearch gs; run_and_record("Greedy", gs, 0); }

        // 3) LOCAL SEARCH
        for (auto mode : { SearchStrategy::LSall, SearchStrategy::BLsmall }) {
            string name = string("LocalSearch-") + (mode == SearchStrategy::LSall ? "LSall" : "BLsmall");
            LocalSearch ls(mode);
            run_and_record(name, ls, 10000);
        }

        // 4/5) AGG & AGE (Práctica 2) – elegir mejor
        double bestP2Fit = -numeric_limits<double>::infinity();
        string bestP2Name;
        // AGG
        for (auto op : { AGGCrossover::CON_ORDEN, AGGCrossover::SIN_ORDEN }) {
            string name = string("AGG-") + (op == AGGCrossover::CON_ORDEN ? "ConOrden" : "SinOrden");
            AGG agg(30, 0.7, 0.1);
            agg.setCrossoverOperator(op);
            double f = run_and_record(name, agg, 1000);
            if (f > bestP2Fit) { bestP2Fit = f; bestP2Name = name; }
        }
        // AGE
        for (auto strat : { CrossoverStrategy::CON_ORDEN, CrossoverStrategy::SIN_ORDEN }) {
            string name = string("AGE-") + (strat == CrossoverStrategy::CON_ORDEN ? "ConOrden" : "SinOrden");
            AGE age(30, 0.1);
            age.setCrossoverStrategy(strat);
            double f = run_and_record(name, age, 1000);
            if (f > bestP2Fit) { bestP2Fit = f; bestP2Name = name; }
        }
        // Registrar Mejor-P2
        txt << "-- Mejor-P2 (" << bestP2Name << ") --\n"
            << "Best fitness: " << bestP2Fit << "\n\n";
        csv << inst << ",Mejor-P2(" << bestP2Name << "),"
            << bestP2Fit << ",,\n";

        // 6) AMs (Práctica 2)
        vector<string> amNames = { "AM-1(All)", "AM-2(Random)", "AM-3(Best)" };
        vector<AMStrategy> amStrats = { AMStrategy::All, AMStrategy::RandomSubset, AMStrategy::BestSubset };
        for (int i = 0; i < 3; ++i) {
            SearchStrategy lsMode = (i == 0 ? SearchStrategy::LSall : SearchStrategy::BLsmall);
            AM am(30, 0.7, 0.1, 0.1, amStrats[i], lsMode);
            run_and_record(amNames[i], am, 1000);
        }

        // 7) ES (P3)./
        { ES es; run_and_record("ES", es, 1000); }

        // 8) BMB (P3)
        { BMB bmb; run_and_record("BMB", bmb, 0); }

        // 9) ILS (P3)
        { ILS ils; run_and_record("ILS", ils, 0); }

        // 9.1) ILS-ES (P3)
        { ILSES ilses; run_and_record("ILS-ES", ilses, 0); }

        // 10) GRASP-NOBL (P3)
        { GRASP g1(GRASP::Mode::NOBL); run_and_record("GRASP-NOBL", g1, 0); }

        // 11) GRASP-SIBL (P3)
        { GRASP g2(GRASP::Mode::SIBL); run_and_record("GRASP-SIBL", g2, 0); }

        cout << "---------------------------------------------\n\n";
        txt << "---------------------------------------------\n\n";
    }

    csv.close();
    txt.close();
    return 0;
}