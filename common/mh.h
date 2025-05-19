#pragma once

#include <problem.h>
#include <utility>
#include <iostream>
#include <ostream>

struct ResultMH {
  tSolution solution;
  tFitness fitness;
  unsigned int evaluations;

  ResultMH(tSolution &sol, tFitness fit, unsigned evals)
      : solution(sol), fitness(fit), evaluations(evals) {}

  /**
   * Imprime el vector solución por std::cout.
   */
  void printSolution() const {
      for (size_t i = 0; i < solution.size(); i++) {
          std::cout << solution[i];
          if(i < solution.size() - 1)
              std::cout << ",";
      }
  }

  /**
   * Imprime el vector solución en el flujo de salida indicado.
   */
  void printSolution(std::ostream &os) const {
      for (size_t i = 0; i < solution.size(); i++) {
          os << solution[i];
          if(i < solution.size() - 1)
              os << ",";
      }
  }
};

/**
 * Interfaz de algoritmo metaheurístico.
 */
class MH {
public:
  virtual ~MH() {}
  virtual ResultMH optimize(Problem *problem, int maxevals) = 0;
};
