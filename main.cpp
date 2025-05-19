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
    // Valores por defecto para prueba rápida
    string instance = "../datos/p2p-Gnutella05.txt";
    long seed = 42;
    int m = 10;
    double p_icm = 0.01;
    int ev_icm = 20;
    string alg = "random";

    if (argc >= 2) instance = argv[1];
    if (argc >= 3) seed = stol(argv[2]);
    if (argc >= 4) m = stoi(argv[3]);
    if (argc >= 5) p_icm = stod(argv[4]);
    if (argc >= 6) ev_icm = stoi(argv[5]);
    if (argc >= 7) alg = argv[6];

    if (argc < 2) {
        cout << "[Modo prueba] Ejecutando con valores por defecto:\n";
        cout << "  ./main " << instance << " " << seed << " " << m << " " << p_icm << " " << ev_icm << " " << alg << endl;
    }

    // Semilla global
    Random::seed(seed);
    cout << "Seed: " << seed << " | m=" << m
         << " | p=" << p_icm << " | ev=" << ev_icm
         << " | alg=" << alg << endl;

    // Cargar problema SNIMP
    ProblemIncrem problem;
    problem.leerArchivo(instance);
    problem.m = m;
    problem.p_icm = p_icm;
    problem.ev_icm = ev_icm;

    // Ejecutar algoritmo
    tSolution emptySol;
    ResultMH result(emptySol, 0.0f, 0);

    auto start = chrono::high_resolution_clock::now();
    if (alg == "random") {
        RandomSearch rs;
        result = rs.optimize(&problem, 10000);
    } else {
        cerr << "Unknown algorithm: " << alg << endl;
        return 1;
    }
    auto end = chrono::high_resolution_clock::now();

    // Mostrar resultados
    cout << "Best fitness: " << result.fitness << endl;
    cout << "Evaluations: " << result.evaluations << endl;
    cout << "Time (s): "
         << chrono::duration<double>(end - start).count() << endl;
    cout << "Solution: [";
    printSolution(result.solution);
    cout << "]\n";

    return 0;
}