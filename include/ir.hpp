#pragma once

#include "ast.hpp"
#include <string>
#include <vector>
#include <ostream>

namespace ir {

enum class OpCode {
    ASSIGN,
    ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT,
    BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, SHL, SHR,
    LABEL, JUMP, JUMP_IF_ZERO, JUMP_IF_NOT_ZERO,
    CALL, PARAM, RETURN,
    LOAD_PTR, STORE_PTR,
    LOAD_MEMBER, STORE_MEMBER
};

struct Instruction {
    OpCode op;
    std::string result;
    std::string arg1;
    std::string arg2;

    std::string toString() const;
};

struct IRFunction {
    std::string name;
    std::vector<Instruction> instructions;
};

struct IRProgram {
    std::vector<IRFunction> functions;
    std::vector<std::string> stringLiterals;

    void print(std::ostream& out) const;
};

class IRGenerator {
public:
    IRProgram generate(const Program& program);

private:
    IRProgram irProg_;
    IRFunction currentFn_;
    int tempCounter_ = 0;
    int labelCounter_ = 0;

    std::string newTemp();
    std::string newLabel(const std::string& prefix = "L");

    void genFunction(const FunctionDecl& fn);
    void genStatement(const Stmt& stmt);
    std::string genExpr(const Expr& expr);
};

} // namespace ir
