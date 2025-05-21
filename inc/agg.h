#ifndef GENETIC_AGG_H
#define GENETIC_AGG_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "genetic_utils.h"
#include <vector>

/// Operadores de cruce disponibles en AGG
enum class AGGCrossover { CON_ORDEN, SIN_ORDEN };

/**
 * AGG: Algoritmo Genético Generacional elitista para SNIMP.
 */
class AGG : public MH, protected GeneticUtils {
public:
    AGG(int popSize = 50, double pc = 0.7, double pm = 0.1);

    void setCrossoverOperator(AGGCrossover op) {
        crossoverOp_ = op;
    }

    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;
    double pc_, pm_;
    AGGCrossover crossoverOp_;
};

#endif // GENETIC_AGG_H
