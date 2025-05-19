#include <pincrem.h>
#include <random.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <numeric>

void ProblemIncrem::leerArchivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("No se pudo abrir: " + filename);

    std::string line;
    int N = 0;
    // Leer cabecera con "Nodes:" y "Edges:"
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '#' && line.find("Nodes:") != std::string::npos) {
            // # Nodes: N Edges: E
            auto pos = line.find("Nodes:");
            std::istringstream iss(line.substr(pos + 6));
            int E;
            iss >> N >> std::ws;
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            iss >> E;
            break;
        }
    }
    if (N <= 0) throw std::runtime_error("Cabecera SNAP inválida");
    size = N;
    adj.assign(N, {});

    // Leer aristas: línea con u<tab>v
    int u, v;
    while (file >> u >> v) {
        if (u >= 0 && v >= 0 && u < N && v < N) {
            // Grafo dirigido: solo de u->v
            adj[u].push_back(v);
        }
    }
}

// Genera solución aleatoria de m nodos únicos
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

// Simulación Independent Cascade (ICM)
double ProblemIncrem::simulateICM(const tSolution &seed_set) {
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
                if (!active[y] && Random::get<double>(0.0, 1.0) < p_icm) {
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

// Fitness SNIMP: minimizamos -spread (maximizamos spread)
tFitness ProblemIncrem::fitness(const tSolution &solution) {
    double spread = simulateICM(solution);
    return -static_cast<tFitness>(spread);
}

// Factoring no usado en SNIMP
SolutionFactoringInfo *ProblemIncrem::generateFactoringInfo(const tSolution&) {
    return nullptr;
}

void ProblemIncrem::updateSolutionFactoringInfo(SolutionFactoringInfo*,
                                                const tSolution&,
                                                unsigned,
                                                tDomain) {
    // noop
}
