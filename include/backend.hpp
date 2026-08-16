#pragma once

#include "ast.hpp"

#include <ostream>

class Backend {
public:
    virtual ~Backend() = default;
    virtual void emit(const Program& program, std::ostream& out) = 0;
};
