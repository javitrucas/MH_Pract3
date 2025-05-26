// src/grasp.h
#pragma once

#include "mh.h"
#include "localsearch.h"
#include "pincrem.h"
#include <vector>

/**
 * GRASP para SNIMP:
 * - GRASP-NOBL: 10 construcciones aleatorizadas, devuelve la mejor sin BL.
 * - GRASP-SIBL: 10 construcciones aleatorizadas + BLsmall sobre cada solución.
 */
class GRASP : public MH {
public:
    enum class Mode { NOBL, SIBL };

    explicit GRASP(Mode mode) : mode_(mode) {}
    virtual ~GRASP() = default;

    /**
     * Ejecuta GRASP:
     * - runs=10 construcciones aleatorizadas.
     * - Si mode_ == SIBL, tras cada construcción aplica BLsmall(maxEvals=100).
     * - Devuelve la mejor solución encontrada y el total de evaluaciones.
     */
    ResultMH optimize(Problem* problem, int maxEvals) override;

private:
    Mode mode_;

    /**
     * Calcula la heurística gne(u) = d+(u) + Σ_{v ∈ N+(u)} d+(v)
     */
    tFitness computeHeuristic(ProblemIncrem* realP, int u);
};
