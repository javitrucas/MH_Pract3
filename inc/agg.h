#ifndef GENETIC_AGG_H
#define GENETIC_AGG_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "genetic_utils.h"
#include <vector>

/// Operadores de cruce disponibles en AGG
enum class AGGCrossover { UNIFORM, POSITION };

/**
 * AGG: Algoritmo Genético Generacional elitista para MDD.
 * - Selección: torneo k=3 repetido popSize veces
 * - Cruce: con probabilidad pc_, estrategia configurable
 * - Mutación: por individuo con probabilidad pm_
 * - Reemplazo: generacional con elitismo
 */
class AGG : public MH, protected GeneticUtils {
public:
    AGG(int popSize = 50, double pc = 0.7, double pm = 0.1);

    /// Configura el operador de cruce
    void setCrossoverOperator(AGGCrossover op) {
        crossoverOp_ = op;
    }

    /// Ejecuta hasta maxEvals evaluaciones
    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;
    double pc_, pm_;
    AGGCrossover crossoverOp_;
};

#endif // GENETIC_AGG_H
