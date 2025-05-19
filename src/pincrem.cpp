#include <pincrem.h>
#include <random.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <string>

// Simulación Independent Cascade (ICM)
static double simulateICM(const std::vector<std::vector<int>>& adj,
                          const std::vector<int>& seed_set,
                          double p, int ev, std::mt19937& rng) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    int total_activated = 0;
    int N = adj.size();
    for (int it = 0; it < ev; ++it) {
        std::vector<char> active(N, 0);
        std::queue<int> Q;
        for (int u : seed_set) {
            active[u] = 1;
            Q.push(u);
        }
        int count = seed_set.size();
        while (!Q.empty()) {
            int u = Q.front(); Q.pop();
            for (int v : adj[u]) {
                if (!active[v] && dist(rng) < p) {
                    active[v] = 1;
                    Q.push(v);
                    ++count;
                }
            }
        }
        total_activated += count;
    }
    return double(total_activated) / ev;
}

// Carga grafo SNAP: ignora comentarios, lee N y E
void ProblemIncrem::leerArchivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("No se pudo abrir: " + filename);

    std::string line;
    // Saltar comentarios hasta cabecera
    int N = 0, E = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        if (iss >> N >> E) break;
    }
    if (N <= 0) throw std::runtime_error("Cabecera SNAP inválida");

    size = N;
    this->m = 0;          // m se establecerá externamente antes de optimize
    adj.assign(N, {});

    int u,v;
    while (file >> u >> v) {
        if (u >= 0 && v >= 0 && u < N && v < N) {
            adj[u].push_back(v);
            adj[v].push_back(u); // grafo no dirigido
        }
    }
}

// Genera solución aleatoria de m nodos únicos
tSolution ProblemIncrem::createSolution() {
    std::vector<int> pool(size);
    std::iota(pool.begin(), pool.end(), 0);
    for (int i = size - 1; i > 0; --i) {
        int j = Random::get<int>(0, i);
        std::swap(pool[i], pool[j]);
    }
    tSolution sol(pool.begin(), pool.begin() + m);
    return sol;
}

// Calcula fitness SNIMP = -spread (queremos maximizar)
tFitness ProblemIncrem::fitness(const tSolution &solution) {
    static thread_local std::mt19937 rng(std::random_device{}());
    double spread = simulateICM(adj, solution, p_icm, ev_icm, rng);
    // convertimos a fitness: valores menores mejores => -spread
    return -static_cast<tFitness>(spread);
}

// Factoring no usado en SNIMP, stubs opcionales
SolutionFactoringInfo *ProblemIncrem::generateFactoringInfo(const tSolution&) {
    return nullptr;
}
void ProblemIncrem::updateSolutionFactoringInfo(SolutionFactoringInfo*,
                                                const tSolution&,
                                                unsigned,
                                                tDomain) {
    // noop
}
