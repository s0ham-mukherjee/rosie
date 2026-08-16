#pragma once

#include "type.hpp"

#include <memory>
#include <string>
#include <vector>

struct Expr {
    virtual ~Expr() = default;
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct FieldDecl {
    Type type;
    std::string name;
    int offset = 0;
};

struct FunctionDecl;

struct StructDecl {
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    size_t sizeBytes = 0;
};

struct IntLiteral : Expr {
    int value;
    explicit IntLiteral(int value) : value(value) {}
};

struct BoolLiteral : Expr {
    bool value;
    explicit BoolLiteral(bool value) : value(value) {}
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string name) : name(std::move(name)) {}
};

struct BinaryExpr : Expr {
    char op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(char op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

struct UnaryExpr : Expr {
    char op;
    std::unique_ptr<Expr> expr;

    UnaryExpr(char op, std::unique_ptr<Expr> expr) : op(op), expr(std::move(expr)) {}
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)), args(std::move(args)) {}
};

struct MethodCallExpr : Expr {
    std::unique_ptr<Expr> object;
    std::string method;
    std::vector<std::unique_ptr<Expr>> args;

    MethodCallExpr(std::unique_ptr<Expr> object, std::string method,
                   std::vector<std::unique_ptr<Expr>> args)
        : object(std::move(object)), method(std::move(method)), args(std::move(args)) {}
};

struct AssignExpr : Expr {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;

    AssignExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : target(std::move(target)), value(std::move(value)) {}
};

struct IndexExpr : Expr {
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index)
        : object(std::move(object)), index(std::move(index)) {}
};

struct MemberExpr : Expr {
    std::unique_ptr<Expr> object;
    std::string member;

    MemberExpr(std::unique_ptr<Expr> object, std::string member)
        : object(std::move(object)), member(std::move(member)) {}
};

struct VarDeclStmt : Stmt {
    Type type;
    std::string name;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(Type type, std::string name, std::unique_ptr<Expr> initializer)
        : type(type), name(std::move(name)), initializer(std::move(initializer)) {}
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch,
           std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;

    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition,
            std::unique_ptr<Expr> increment, std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct FunctionDecl {
    Type returnType;
    std::string name;
    std::string structName;
    std::vector<std::pair<Type, std::string>> params;
    std::unique_ptr<BlockStmt> body;
    bool isMethod = false;
};

struct FloatLiteral : Expr {
    double value;
    explicit FloatLiteral(double value) : value(value) {}
};

struct CharLiteral : Expr {
    char value;
    explicit CharLiteral(char value) : value(value) {}
};

struct StringLiteral : Expr {
    std::string value;
    explicit StringLiteral(std::string value) : value(std::move(value)) {}
};

struct TernaryExpr : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;

    TernaryExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> thenExpr,
                std::unique_ptr<Expr> elseExpr)
        : condition(std::move(condition)),
          thenExpr(std::move(thenExpr)),
          elseExpr(std::move(elseExpr)) {}
};

struct DoWhileStmt : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;

    DoWhileStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> condition)
        : body(std::move(body)), condition(std::move(condition)) {}
};

struct BreakStmt : Stmt {};

struct ContinueStmt : Stmt {};

struct CaseBranch {
    int value = 0;
    bool isDefault = false;
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct SwitchStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<CaseBranch> cases;

    SwitchStmt(std::unique_ptr<Expr> condition, std::vector<CaseBranch> cases)
        : condition(std::move(condition)), cases(std::move(cases)) {}
};

struct NamespaceDecl {
    std::string name;
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    std::vector<std::unique_ptr<StructDecl>> structs;
};

// Expanded Program node containing functions, structs, classes, namespaces
struct Program {
    std::vector<std::unique_ptr<StructDecl>> structs;
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    std::vector<std::unique_ptr<NamespaceDecl>> namespaces;
};

class ASTPrinter {
public:
    static void print(const Program& program, std::ostream& out);
};
