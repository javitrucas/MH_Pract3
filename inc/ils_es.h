#pragma once

#include "mh.h"
#include "es.h"
#include "pincrem.h"
#include <vector>

/**
 * Hibridación ILS-ES para SNIMP
 * - 10 iteraciones: aplicar ES en S0 aleatoria, luego mutación + ES.
 * - Mutación: cambiar el 20% de componentes (mínimo 2) a valores no seleccionados.
 */
class ILSES : public MH {
public:
    ILSES() = default;
    virtual ~ILSES() = default;
    ResultMH optimize(Problem* problem, int maxEvals) override;
};
