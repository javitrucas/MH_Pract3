#ifndef AGE_H
#define AGE_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "genetic_utils.h"
#include <vector>

/// Opciones de cruce para AGE
enum class CrossoverStrategy { UNIFORM, POSITION };

/**
 * AGE: Algoritmo Genético Estacionario con BL integrado.
 */
class AGE : public MH, protected GeneticUtils {
public:
    AGE(int popSize = 50, double pm = 0.1);

    /// Establece cruce uniforme o por posición
    void setCrossoverStrategy(CrossoverStrategy strat) {
        crossoverStrategy_ = strat;
    }

    /// Ejecuta hasta maxEvals evaluaciones
    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;
    double pm_;
    CrossoverStrategy crossoverStrategy_;
};

#endif // AGE_H
