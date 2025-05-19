#include <pincrem.h>
#include <random.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <numeric>
#include <limits>
#include <random>

/**
 * Carga una instancia SNIMP en formato SNAP:
 * - Ignora comentarios.
 * - Lee "# Nodes: N Edges: E" para obtener número de nodos.
 * - Cada línea u v añade arista dirigida u->v.
 */
void ProblemIncrem::leerArchivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("No se pudo abrir: " + filename);

    std::string line;
    size = 0;

    // 1) Buscar línea con "# Nodes: N Edges: E"
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '#' && line.find("Nodes:") != std::string::npos) {
            std::istringstream iss(line);
            std::string word;
            while (iss >> word) {
                if (word == "Nodes:") iss >> size;
                else if (word == "Edges:") { int E; iss >> E; break; }
            }
            break;
        }
    }
    if (size <= 0) throw std::runtime_error("Cabecera SNAP inválida");
    adj.assign(size, {});

    // 2) Leer pares (u v) como aristas dirigidas u->v
    int u, v;
    int edge_count = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        if (iss >> u >> v) {
            if (u >= 0 && v >= 0 && u < size && v < size) {
                adj[u].push_back(v);
                ++edge_count;
            }
        }
    }
    std::cout << "[INFO] Cargado grafo con " << size << " nodos y " << edge_count << " aristas.\n";
}

/**
 * Genera solución aleatoria (seed set) de m nodos únicos.
 */
tSolution ProblemIncrem::createSolution() {
    tSolution sol(size);
    std::iota(sol.begin(), sol.end(), 0);
    for (int i = size - 1; i > 0; --i) {
        int j = Random::get<int>(0, i);
        std::swap(sol[i], sol[j]);
    }
    sol.resize(m);
    return sol;
}

/**
 * Simula el Independent Cascade Model sobre un seed set.
 * Devuelve promedio de nodos activados en ev iteraciones.
 */
double ProblemIncrem::simulateICM(const tSolution &seed_set) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    static thread_local std::mt19937 rng(std::random_device{}());
    int total = 0;

    for (int it = 0; it < ev_icm; ++it) {
        std::vector<char> active(size, 0);
        std::queue<int> Q;
        for (int u : seed_set) {
            active[u] = 1;
            Q.push(u);
        }
        int count = seed_set.size();
        while (!Q.empty()) {
            int x = Q.front(); Q.pop();
            for (int y : adj[x]) {
                double r = dist(rng);
                if (!active[y] && r < p_icm) {
                    active[y] = 1;
                    Q.push(y);
                    ++count;
                }
            }
        }
        total += count;
    }
    return double(total) / ev_icm;
}

/**
 * Fitness SNIMP: minimizamos -spread para maximizar el spread real.
 */
tFitness ProblemIncrem::fitness(const tSolution &solution) {
    double spread = simulateICM(solution);
    return -static_cast<tFitness>(spread);
}

// Factoring no usado
SolutionFactoringInfo *ProblemIncrem::generateFactoringInfo(const tSolution&) {
    return nullptr;
}

void ProblemIncrem::updateSolutionFactoringInfo(SolutionFactoringInfo*,
                                                const tSolution&,
                                                unsigned,
                                                tDomain) {
    // noop
}
