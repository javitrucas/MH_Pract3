#pragma once

#include "mh.h"
#include "localsearch.h"

/**
 * Búsqueda Multiarranque Básica (BMB) para SNIMP
 * Genera 10 soluciones aleatorias iniciales y aplica BLsmall a cada una.
 * Devuelve la mejor solución encontrada.
 */
class BMB : public MH {
public:
    BMB() = default;
    virtual ~BMB() = default;

    /**
     * Ejecuta la BMB:
     * - 10 iteraciones: crear solución aleatoria, evaluarla,
     *   aplicar BLsmall (SearchStrategy::BLsmall) con maxEvals=100.
     * - Acumula el número total de evaluaciones.
     */
    ResultMH optimize(Problem* problem, int maxEvals) override;
};
