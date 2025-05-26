#pragma once

#include "mh.h"
#include "localsearch.h"
#include "pincrem.h"
#include <vector>

/**
 * Búsqueda Local Reiterada (ILS) para SNIMP
 * - 10 iteraciones: aplicar BLsmall en S0 aleatoria, luego mutación + BLsmall.
 * - Mutación: cambiar el 20% de componentes (mínimo 2) a valores no seleccionados.
 */
class ILS : public MH {
public:
    ILS() = default;
    virtual ~ILS() = default;
    ResultMH optimize(Problem* problem, int maxEvals) override;
};
