#ifndef AGE_H
#define AGE_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "genetic_utils.h"
#include <vector>

/// Opciones de cruce para AGE
enum class CrossoverStrategy { CON_ORDEN, SIN_ORDEN };

/**
 * AGE: Algoritmo Genético Estacionario
 */
class AGE : public MH, protected GeneticUtils {
public:
    AGE(int popSize = 50, double pm = 0.1);

    void setCrossoverStrategy(CrossoverStrategy strat) {
        crossoverStrategy_ = strat;
    }

    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;
    double pm_;
    CrossoverStrategy crossoverStrategy_;
};

#endif // AGE_H
