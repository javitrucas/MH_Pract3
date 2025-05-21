#ifndef AM_H
#define AM_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "localsearch.h"
#include "genetic_utils.h"
#include <vector>

enum class AMStrategy {
    All,           // BL a TODOS los hijos
    RandomSubset,  // BL a un subset aleatorio
    BestSubset     // BL a los mejores hijos
};

enum class AMCrossover {
    CON_ORDEN,
    SIN_ORDEN
};

class AM : public MH, protected GeneticUtils {
public:
    AM(int popSize,
       double pc,
       double pm,
       double proportion,
       AMStrategy strategy,
       SearchStrategy lsStrategy = SearchStrategy::LSall);

    void setCrossoverOperator(AMCrossover op) {
        crossoverOp_ = op;
    }

    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;
    double pc_, pm_;
    double proportion_;
    AMStrategy strategy_;
    AMCrossover crossoverOp_;
    SearchStrategy lsStrategy_;
    int maxLSIters_;  // nº de evaluaciones a pasarle a la BL en cada llamada
};

#endif // AM_H
