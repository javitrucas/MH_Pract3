// ===== am.h =====

#ifndef AM_H
#define AM_H

#include "mh.h"
#include "problem.h"
#include "pincrem.h"
#include "localsearch.h"  // Para usar randLS
#include "genetic_utils.h" // Para operadores genéticos
#include <vector>

// AM: Tipos de aplicación de Búsqueda Local
enum class AMStrategy { 
    All,            // 100% individuos
    RandomSubset,   // 10% random
    BestSubset      // 10% mejores
};

// Clase AM: Algoritmo Memético
class AM : public MH, protected GeneticUtils {
public:
    AM(int popSize, double pc, double pm,
        double proportion, AMStrategy strategy,
        SearchStrategy lsStrategy = SearchStrategy::randLS);

    virtual ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    int popSize_;           ///< Tamaño población
    double pc_;             ///< Probabilidad de cruce
    double pm_;             ///< Probabilidad de mutación
    double proportion_;     ///< Proporción de individuos a los que aplicar BL
    AMStrategy strategy_;   ///< Tipo de estrategia BL (all, random, best)
    SearchStrategy lsStrategy_;
};

#endif // AM_H
