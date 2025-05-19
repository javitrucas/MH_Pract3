#include <iostream>
#include <string>
#include <chrono>
#include "pincrem.h"
#include "randomsearch.h"
#include "greedy.h"
#include "localsearch.h"
#include <random.hpp>

using namespace std;

void printSolution(const tSolution &sol) {
    for (size_t i = 0; i < sol.size(); ++i) {
        cout << sol[i] << (i + 1 < sol.size() ? ", " : "");
    }
}

int main(int argc, char *argv[]) {
    // Parámetros comunes
    long seed = 42;
    int m = 10;
    double p_icm = 0.01;
    int ev_icm = 20;

    // Lista de instancias SNAP
    vector<string> instances = {
        "../datos/ca-GrQc.txt",
        "../datos/p2p-Gnutella05.txt",
        "../datos/p2p-Gnutella08.txt",
        "../datos/p2p-Gnutella25.txt"
    };
    vector<string> algs = {"random", "greedy", "heurLS", "randLS"};

    // Semilla global
    Random::seed(seed);
    cout << "Batch SNIMP: seed=" << seed << " m=" << m
         << " p=" << p_icm << " ev=" << ev_icm << endl;

    // Para cada instancia y algoritmo
    for (const auto &inst : instances) {
        cout << "\n=== Instancia: " << inst << " ===" << endl;
        ProblemIncrem problem;
        try {
            problem.leerArchivo(inst);
        } catch (exception &e) {
            cerr << "ERROR cargando " << inst << ": " << e.what() << endl;
            continue;
        }
        problem.m = m;
        problem.p_icm = p_icm;
        problem.ev_icm = ev_icm;

        for (const auto &alg : algs) {
            cout << "-- Algoritmo: " << alg << " --" << endl;
            tSolution emptySol;
            ResultMH result(emptySol, 0.0f, 0);
            auto start = chrono::high_resolution_clock::now();
            if (alg == "random") {
                RandomSearch rs;
                result = rs.optimize(&problem, 10000);
            } else if (alg == "greedy") {
                GreedySearch gs;
                result = gs.optimize(&problem, 0);
            } else if (alg == "heurLS") {
                LocalSearch ls(SearchStrategy::heurLS);
                result = ls.optimize(&problem, 10000);
            } else if (alg == "randLS") {
                LocalSearch ls2(SearchStrategy::randLS);
                result = ls2.optimize(&problem, 10000);
            } else {
                cerr << "ERROR: Algoritmo desconocido: " << alg << endl;
                continue;
            }
            auto end = chrono::high_resolution_clock::now();

            cout << "Best fitness: " << result.fitness;
            cout << " | Eval: " << result.evaluations;
            cout << " | Time (s): "
                 << chrono::duration<double>(end - start).count() << endl;
            cout << "Solution: [";
            printSolution(result.solution);
            cout << "]\n";
        }
    }

    return 0;
}
