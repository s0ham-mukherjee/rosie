#pragma once

#include "ast.hpp"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class CodeGenerator {
public:
    void emit(const Program& program, std::ostream& out);

private:
    std::ostream* out_ = nullptr;
    int labelCounter_ = 0;
    std::unordered_map<std::string, int> locals_;
    int stackOffset_ = 0;
    std::vector<std::unordered_map<std::string, int>> localScopes_;
    std::vector<std::string> breakTargets_;
    std::vector<std::string> continueTargets_;

    std::string freshLabel(const std::string& prefix);
    void emitFunctionPrologue(const FunctionDecl& fn);
    void emitFunctionEpilogue();
    void emitStatement(const Stmt& stmt, const std::string& currentFunction = "main");
    void emitExpression(const Expr& expr);
    void pushRegister(const char* reg);
    void popToRegister(const char* reg);

    void beginScope();
    void endScope();
    int allocateLocal(const std::string& name);
    int lookupLocal(const std::string& name) const;
};
