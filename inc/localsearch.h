#pragma once
#include <mh.h>


enum class SearchStrategy {
  randLS,
  heurLS 
};

class LocalSearch : public MH {
private:
    SearchStrategy explorationMode;

public:
    explicit LocalSearch(SearchStrategy mode) : MH(), explorationMode(mode) {}
    virtual ~LocalSearch() = default;
    ResultMH optimize(Problem* problem, int maxIterations) override;
};