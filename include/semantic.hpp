#pragma once

#include "ast.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class SemanticError : public std::runtime_error {
public:
    explicit SemanticError(const std::string& message) : std::runtime_error(message) {}
};

struct FunctionSymbol {
    Type returnType;
    std::vector<Type> paramTypes;
    bool isMethod = false;
    std::string structName;
};

class SemanticAnalyzer {
public:
    void analyze(Program& program);

    const std::unordered_map<std::string, StructDecl*>& structs() const { return structs_; }

private:
    Program* program_ = nullptr;
    std::unordered_map<std::string, FunctionSymbol> functions_;
    std::unordered_map<std::string, StructDecl*> structs_;
    std::vector<std::unordered_map<std::string, Type>> scopes_;
    std::string currentStruct_;
    bool inMethod_ = false;

    void computeStructLayout(StructDecl& decl);
    void registerFunctions(Program& program);
    void analyzeFunction(FunctionDecl& fn);
    void analyzeMethod(FunctionDecl& fn, StructDecl& owner);
    void analyzeBlock(BlockStmt& block, Type returnType);
    void analyzeStatement(Stmt& stmt, Type returnType);
    Type analyzeExpression(Expr& expr);

    Type analyzeLValue(Expr& expr);
    void declareVariable(const std::string& name, Type type);
    Type resolveVariable(const std::string& name) const;
    Type resolveField(const Type& objectType, const std::string& member) const;
    bool typesCompatible(const Type& target, const Type& value) const;
    void beginScope();
    void endScope();

    std::string mangleMethod(const std::string& structName, const std::string& method) const;
    SemanticError error(const std::string& message) const;
};
