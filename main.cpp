#include <iostream>
#include <problem.h>
#include <random.hpp>
#include <string>
#include <util.h>
#include <filesystem>
#include "pincrem.h"
#include "agg.h"   // AGG
#include "age.h"   // AGE
#include "am.h"    // AM
#include <chrono>  // Para medir el tiempo

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    // 1) Parámetros de entrada
    string instance_path;
    long int seed;

    if (argc < 3) {
        cout << "Usage: " << argv[0]
             << " <path_to_instance_file> <seed>" << endl;
        instance_path = "../datos_MDD/GKD-b_6_n25_m7.txt";
        seed = 42;
    } else {
        instance_path = argv[1];
        seed = atol(argv[2]);
    }

    // 2) Semilla
    Random::seed(seed);
    cout << "Using fixed random seed: " << seed << endl;

    // 3) Cargar instancia
    cout << "Loading problem instance from: " << instance_path << endl;
    ProblemIncrem problem;
    problem.leerArchivo(instance_path);
    cout << "Instance: " << fs::path(instance_path).stem().string() << endl;

    // ------- AGG -------
    cout << "\n*** Running AGG (Generational with Elitism) ***\n";
    vector<AGGCrossover> aggOps = { AGGCrossover::UNIFORM, AGGCrossover::POSITION };
    for (auto op : aggOps) {
        AGG agg(50, 0.7, 0.1);
        agg.setCrossoverOperator(op);

        cout << "\n=== AGG ("
             << (op == AGGCrossover::UNIFORM ? "Uniform" : "Position")
             << ") ===" << endl;

        auto start = chrono::high_resolution_clock::now();
        ResultMH res = agg.optimize(&problem, 100000);
        auto end = chrono::high_resolution_clock::now();

        cout << "Best fitness: " << res.fitness << endl;
        cout << "Evaluations: " << res.evaluations << endl;
        cout << "Time: " << chrono::duration<double>(end - start).count() << " s" << endl;
        cout << "Solution: [";
        res.printSolution();
        cout << "]\n";
    }

    // ------- AGE -------
    cout << "\n*** Running AGE (Steady-State) ***\n";
    vector<CrossoverStrategy> ageStrats = { CrossoverStrategy::UNIFORM, CrossoverStrategy::POSITION };
    for (auto strat : ageStrats) {
        AGE age(50, 0.1);
        age.setCrossoverStrategy(strat);

        cout << "\n=== AGE ("
             << (strat == CrossoverStrategy::UNIFORM ? "Uniform" : "Position")
             << ") ===" << endl;

        auto start = chrono::high_resolution_clock::now();
        ResultMH res = age.optimize(&problem, 100000);
        auto end = chrono::high_resolution_clock::now();

        cout << "Best fitness: " << res.fitness << endl;
        cout << "Evaluations: " << res.evaluations << endl;
        cout << "Time: " << chrono::duration<double>(end - start).count() << " s" << endl;
        cout << "Solution: [";
        res.printSolution();
        cout << "]\n";
    }

    // ------- AM (Meméticos) -------
    cout << "\n*** Running AM (Memetic Algorithms) ***\n";

    vector<AMStrategy> amStrats  = { AMStrategy::All,       AMStrategy::RandomSubset,  AMStrategy::BestSubset };
    vector<string>    amNames   = { "All (AM-1)",          "RandomSubset (AM-2)",     "BestSubset (AM-3)" };
    vector<SearchStrategy> lsModes = { SearchStrategy::randLS, SearchStrategy::heurLS };
    vector<string>          lsNames = { "RandLS",               "HeurLS" };

    for (size_t m = 0; m < lsModes.size(); ++m) {
        cout << "\n--- Using Local Search: " << lsNames[m] << " ---\n";
        for (size_t i = 0; i < amStrats.size(); ++i) {
            // popSize=50, pc=0.7, pm=0.1, pLS=0.1
            AM am(50, 0.7, 0.1, 0.1, amStrats[i], lsModes[m]);

            cout << "\n=== AM " << amNames[i]
                 << " + " << lsNames[m]
                 << " ===" << endl;

            auto start = chrono::high_resolution_clock::now();
            ResultMH res = am.optimize(&problem, 100000);
            auto end = chrono::high_resolution_clock::now();

            cout << "Best fitness: " << res.fitness << endl;
            cout << "Evaluations: " << res.evaluations << endl;
            cout << "Time: " << chrono::duration<double>(end - start).count() << " s" << endl;
            cout << "Solution: [";
            res.printSolution();
            cout << "]\n";
        }
    }

    return 0;
}
