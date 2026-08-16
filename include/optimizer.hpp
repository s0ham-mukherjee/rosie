#pragma once

#include "ir.hpp"

namespace ir {

class Optimizer {
public:
    explicit Optimizer(int level = 1) : level_(level) {}

    void optimize(IRProgram& program);

private:
    int level_;

    void optimizeFunction(IRFunction& fn);
    bool foldConstants(IRFunction& fn);
    bool algebraicSimplification(IRFunction& fn);
    bool eliminateDeadCode(IRFunction& fn);
};

} // namespace ir
