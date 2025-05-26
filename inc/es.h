#pragma once

#include "mh.h"
#include "pincrem.h"
#include <unordered_set>
#include <vector>

class ES : public MH {
public:
    ES() = default;
    virtual ~ES() = default;

    ResultMH optimize(Problem* problem, int maxEvals) override;
};
