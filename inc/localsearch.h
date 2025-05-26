#pragma once
#include <mh.h>

enum class SearchStrategy {
  LSall,    // Hasta 1000 evaluaciones
  BLsmall   // Hasta 1000 evaluaciones o 20 sin mejora
};

class LocalSearch : public MH {
private:
    SearchStrategy explorationMode;

public:
    explicit LocalSearch(SearchStrategy mode) 
      : MH(), explorationMode(mode) {}
    virtual ~LocalSearch() = default;
    ResultMH optimize(Problem* problem, int maxIterations) override;
};
