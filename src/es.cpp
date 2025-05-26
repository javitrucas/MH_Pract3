#include "es.h"
#include <cmath>
#include <random.hpp>
#include <algorithm>
#include <cassert>

using namespace std;

ResultMH ES::optimize(Problem* problem, int maxEvals) {
    // Parámetros fijos
    const double phi = 0.3;
    const double mu  = 0.2;
    const double Tf  = 1e-3;

    // Datos del problema
    auto* realP = dynamic_cast<ProblemIncrem*>(problem);
    size_t n    = problem->getSolutionSize();
    tSolution sol    = problem->createSolution();
    size_t m    = sol.size();

    // Fitness inicial
    tFitness fSol  = problem->fitness(sol);
    tSolution bestSol = sol;
    tFitness fBest = fSol;

    int evals = 1;
    int maxVecinos = 5 * m;
    int maxExitos  = max(1, maxVecinos / 10);

    // Número de enfriamientos
    int M = maxEvals / maxVecinos;

    // Temperatura inicial y coeficiente beta
    double T0 = (mu * fSol) / (-log(phi));
    if (T0 <= Tf) T0 = Tf * 10;
    double beta = (T0 - Tf) / (M * T0 * Tf);
    double T = T0;

    // Prepara vectores de selección rápida
    vector<int> posOf(n, -1);
    vector<int> selVec, nonSelVec;
    selVec.reserve(m); nonSelVec.reserve(n - m);

    for (int i = 0; i < (int)m; ++i) {
        selVec.push_back(sol[i]);
        posOf[sol[i]] = i;
    }
    for (int i = 0; i < (int)n; ++i) {
        if (posOf[i] == -1) nonSelVec.push_back(i);
    }

    // Bucle de enfriamientos
    while (evals < maxEvals && T > Tf) {
        int exitos = 0, vecinos = 0;

        // Bucle interior L(T)
        while (vecinos < maxVecinos && exitos < maxExitos && evals < maxEvals) {
            // Elegir aleatoriamente posición a modificar
            int pos = Random::get<int>(0, m - 1);

            // Escoger valor no seleccionado aleatorio
            int idxNon = Random::get<int>(0, (int)nonSelVec.size() - 1);
            int v = nonSelVec[idxNon];
            int u = sol[pos];

            // Generar vecino in-place
            sol[pos] = v;
            tFitness fNeighbor = problem->fitness(sol);
            ++evals;
            ++vecinos;

            // Criterio de aceptación
            double delta = fSol - fNeighbor;
            bool accept = (delta < 0) ||
                          (Random::get<double>(0.0, 1.0) <= exp(-delta / T));

            if (accept) {
                // Aceptar vecino
                fSol = fNeighbor;
                ++exitos;

                // Actualizar estructuras posOf, selVec y nonSelVec
                selVec[pos] = v;
                posOf[v]    = pos;
                posOf[u]    = -1;
                nonSelVec[idxNon] = u;

                // Actualizar mejor global
                if (fSol > fBest) {
                    fBest = fSol;
                    bestSol = sol;
                }
            } else {
                // Revertir
                sol[pos] = u;
            }
        }

        // Si no hubo éxitos, terminamos
        if (exitos == 0) break;
        // Enfriamiento Cauchy modificado
        T = T / (1 + beta * T);
    }

    return ResultMH(bestSol, fBest, evals);
}
