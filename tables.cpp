// tables.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <filesystem>
#include <vector>
#include <algorithm>
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

struct Metrics {
    double fitness;
    int evals;
    double time;
};

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

    fs::create_directories("../output");
    ofstream csv("../output/results.csv");
    csv << "Instancia,Algoritmo,Position,Fitness,Evals,Time\n";
    ofstream txt("../output/results.txt");

    Random::seed(seed);
    cout << "Batch SNIMP:\n"
         << "  seed=" << seed
         << " | m=" << m
         << " | p_icm=" << p_icm
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

        struct ResultRow {
            string name;
            Metrics metrics;
        };

        vector<ResultRow> rows;

        auto run_and_capture = [&](const string &name, MH &alg, int maxEvals=0) {
            auto start = chrono::high_resolution_clock::now();
            auto res   = alg.optimize(&problem, maxEvals);
            auto end   = chrono::high_resolution_clock::now();
            double t   = chrono::duration<double>(end - start).count();
            cout << "-- " << name << " --\n"
                 << "Best fitness: " << res.fitness
                 << " | evals=" << res.evaluations
                 << " | t(s)=" << fixed << setprecision(4) << t << "\n"
                 << "Solution: ["; printSolution(res.solution); cout << "]\n\n";
            txt << "-- " << name << " --\n"
                << "Best fitness: " << res.fitness
                << " | evals=" << res.evaluations
                << " | t(s)=" << fixed << setprecision(4) << t << "\n"
                << "Solution: [";
            for (size_t i = 0; i < res.solution.size(); ++i)
                txt << res.solution[i] << (i + 1 < res.solution.size() ? ", " : "");
            txt << "]\n\n";
            rows.push_back({name, {res.fitness, res.evaluations, t}});
        };

        // Ejecutar algoritmos base
        { RandomSearch rs; run_and_capture("RandomSearch", rs, 10000); }
        { GreedySearch gs; run_and_capture("Greedy", gs, 0); }
        for (auto mode : { SearchStrategy::LSall, SearchStrategy::BLsmall }) {
            string name = string("LocalSearch-") + (mode==SearchStrategy::LSall?"LSall":"BLsmall");
            LocalSearch ls(mode);
            run_and_capture(name, ls, 10000);
        }

        // Inicializamos mejor de Práctica 2 (AGG, AGE, AM)
        Metrics bestP2{ -numeric_limits<double>::infinity(), 0, 0.0 };
        string bestP2Name;

        // AGG
        for (auto op : { AGGCrossover::CON_ORDEN, AGGCrossover::SIN_ORDEN }) {
            string name = string("AGG-") + (op==AGGCrossover::CON_ORDEN?"ConOrden":"SinOrden");
            AGG agg(30,0.7,0.1); agg.setCrossoverOperator(op);
            run_and_capture(name, agg, 1000);
            if (rows.back().metrics.fitness > bestP2.fitness) {
                bestP2 = rows.back().metrics;
                bestP2Name = name;
            }
        }
        // AGE
        for (auto strat : { CrossoverStrategy::CON_ORDEN, CrossoverStrategy::SIN_ORDEN }) {
            string name = string("AGE-") + (strat==CrossoverStrategy::CON_ORDEN?"ConOrden":"SinOrden");
            AGE age(30,0.1); age.setCrossoverStrategy(strat);
            run_and_capture(name, age, 1000);
            if (rows.back().metrics.fitness > bestP2.fitness) {
                bestP2 = rows.back().metrics;
                bestP2Name = name;
            }
        }
        // AM
        { AM am1(30,0.7,0.1,0.1,AMStrategy::All,     SearchStrategy::LSall);     run_and_capture("AM-1(All)", am1, 1000);
            if (rows.back().metrics.fitness > bestP2.fitness) {
                bestP2 = rows.back().metrics;
                bestP2Name = "AM-1(All)";
            }
        }
        { AM am2(30,0.7,0.1,0.1,AMStrategy::RandomSubset, SearchStrategy::BLsmall); run_and_capture("AM-2(Random)", am2, 1000);
            if (rows.back().metrics.fitness > bestP2.fitness) {
                bestP2 = rows.back().metrics;
                bestP2Name = "AM-2(Random)";
            }
        }
        { AM am3(30,0.7,0.1,0.1,AMStrategy::BestSubset,   SearchStrategy::BLsmall); run_and_capture("AM-3(Best)", am3, 1000);
            if (rows.back().metrics.fitness > bestP2.fitness) {
                bestP2 = rows.back().metrics;
                bestP2Name = "AM-3(Best)";
            }
        }

        // Imprimir mejor de Práctica 2
        txt << "-- Mejor-P2 (" << bestP2Name << ") --\n"
            << "Best fitness: " << bestP2.fitness
            << " | evals=" << bestP2.evals
            << " | t(s)=" << fixed << setprecision(4) << bestP2.time << "\n\n";
        rows.push_back({string("Mejor-P2("+bestP2Name+")"), bestP2});

        // Resto de metaheurísticas
        { ES es;      run_and_capture("ES", es, 1000); }
        { BMB bmb;    run_and_capture("BMB", bmb, 0);   }
        { ILS ils;    run_and_capture("ILS", ils, 0);   }
        { ILSES ilses; run_and_capture("ILS-ES", ilses, 0); }
        { GRASP g1(GRASP::Mode::NOBL); run_and_capture("GRASP-NOBL", g1, 0); }
        { GRASP g2(GRASP::Mode::SIBL); run_and_capture("GRASP-SIBL", g2, 0); }

        // Ordenar por fitness y calcular posiciones
        vector<size_t> idx(rows.size());
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](size_t a, size_t b) {
            return rows[a].metrics.fitness > rows[b].metrics.fitness;
        });

        vector<int> positions(rows.size());
        for (size_t i = 0; i < idx.size(); ++i)
            positions[idx[i]] = i + 1;

        // Escribir CSV
        for (size_t i = 0; i < rows.size(); ++i) {
            csv << inst << ","
                << rows[i].name << ","
                << positions[i] << ","
                << rows[i].metrics.fitness << ","
                << rows[i].metrics.evals << ","
                << rows[i].metrics.time << "\n";
        }

        cout << "---------------------------------------------\n\n";
        txt  << "---------------------------------------------\n\n";
    }

    csv.close();
    txt.close();
    return 0;
}
