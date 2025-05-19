#pragma once
#include <algorithm>
#include <problem.h>


/**
 * Clase que almacena información para evaluación factorada.
 */
class ProblemIncremFactoringInfo : public SolutionFactoringInfo {
  public:
    std::vector<tFitness> fitness_parcial; // Almacena los valores parciales de la solución
  
    ProblemIncremFactoringInfo(size_t m) : fitness_parcial(m, 0) {}
    ProblemIncremFactoringInfo() {}
  };

class ProblemIncrem : public Problem {
  size_t size;                // Número de elementos (n)

private:
  int m;                        // Número de elementos seleccionados (m)
  std::vector<std::vector<float>> D;  // Matriz de distancias

public:
  ProblemIncrem(size_t size) : Problem() { this->size = size; }
  ProblemIncrem() : Problem() {size=0; m=0;}
  tFitness fitness(const tSolution &solution) override;
  tSolution createSolution() override;
  size_t getSolutionSize() override { return size; }
  std::pair<tDomain, tDomain> getSolutionDomainRange() override {
    return std::make_pair(false, true);
  }
  int getM() { return m; }

  //vector<tHeuristic> heuristic(tSolution &sol, const vector<tOption> &options);

  SolutionFactoringInfo *generateFactoringInfo(const tSolution &solution) override;

  void updateSolutionFactoringInfo(SolutionFactoringInfo *solution_info,
                                    const tSolution &solution,
                                    unsigned pos_change,
                                    tDomain new_value) override;

  void leerArchivo(const std::string& filename);
};
