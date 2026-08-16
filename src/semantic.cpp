#include "semantic.hpp"

void SemanticAnalyzer::analyze(Program& program) {
    program_ = &program;
    functions_.clear();
    structs_.clear();

    for (auto& decl : program.structs) {
        computeStructLayout(*decl);
        structs_[decl->name] = decl.get();
    }

    registerFunctions(program);

    bool hasMain = false;
    for (auto& fn : program.functions) {
        if (fn->name == "main") {
            hasMain = true;
        }
        analyzeFunction(*fn);
    }

    for (auto& decl : program.structs) {
        for (auto& method : decl->methods) {
            analyzeMethod(*method, *decl);
        }
    }

    if (!hasMain) {
        throw error("program must define int main()");
    }
}

void SemanticAnalyzer::computeStructLayout(StructDecl& decl) {
    int offset = 0;
    for (auto& field : decl.fields) {
        field.offset = offset;
        field.type.structDef = structs_.count(field.type.structName) ? structs_[field.type.structName] : field.type.structDef;
        offset += static_cast<int>(field.type.sizeBytes());
    }
    decl.sizeBytes = static_cast<size_t>(offset);
}

void SemanticAnalyzer::registerFunctions(Program& program) {
    for (auto& fn : program.functions) {
        FunctionSymbol symbol;
        symbol.returnType = fn->returnType;
        for (const auto& param : fn->params) {
            symbol.paramTypes.push_back(param.first);
        }
        if (functions_.count(fn->name)) {
            throw error("duplicate function '" + fn->name + "'");
        }
        functions_[fn->name] = symbol;
    }

    for (auto& decl : program.structs) {
        for (auto& method : decl->methods) {
            FunctionSymbol symbol;
            symbol.returnType = method->returnType;
            symbol.isMethod = true;
            symbol.structName = decl->name;
            symbol.paramTypes.push_back(Type::makePointer(Type::makeStruct(decl->name, decl.get())));
            for (const auto& param : method->params) {
                symbol.paramTypes.push_back(param.first);
            }
            std::string mangled = mangleMethod(decl->name, method->name);
            if (functions_.count(mangled)) {
                throw error("duplicate method '" + method->name + "' in struct '" + decl->name + "'");
            }
            functions_[mangled] = symbol;
        }
    }
}

void SemanticAnalyzer::analyzeFunction(FunctionDecl& fn) {
    if (fn.returnType.isVoid() && fn.name == "main") {
        throw error("main must return int");
    }

    currentStruct_.clear();
    inMethod_ = false;
    beginScope();
    for (const auto& param : fn.params) {
        declareVariable(param.second, param.first);
    }
    analyzeBlock(*fn.body, fn.returnType);
    endScope();
}

void SemanticAnalyzer::analyzeMethod(FunctionDecl& fn, StructDecl& owner) {
    currentStruct_ = owner.name;
    inMethod_ = true;
    beginScope();
    declareVariable("this", Type::makePointer(Type::makeStruct(owner.name, &owner)));
    for (const auto& field : owner.fields) {
        declareVariable(field.name, field.type);
    }
    for (const auto& param : fn.params) {
        declareVariable(param.second, param.first);
    }
    analyzeBlock(*fn.body, fn.returnType);
    endScope();
    inMethod_ = false;
    currentStruct_.clear();
}

void SemanticAnalyzer::analyzeBlock(BlockStmt& block, Type returnType) {
    beginScope();
    for (auto& stmt : block.statements) {
        analyzeStatement(*stmt, returnType);
    }
    endScope();
}

void SemanticAnalyzer::analyzeStatement(Stmt& stmt, Type returnType) {
    if (auto* varDecl = dynamic_cast<VarDeclStmt*>(&stmt)) {
        if (varDecl->type.isVoid()) {
            throw error("cannot declare variable of type void");
        }
        if (varDecl->initializer) {
            Type initType = analyzeExpression(*varDecl->initializer);
            if (!typesCompatible(varDecl->type, initType)) {
                throw error("initializer type mismatch for '" + varDecl->name + "'");
            }
        }
        declareVariable(varDecl->name, varDecl->type);
        return;
    }

    if (auto* exprStmt = dynamic_cast<ExprStmt*>(&stmt)) {
        analyzeExpression(*exprStmt->expr);
        return;
    }

    if (auto* returnStmt = dynamic_cast<ReturnStmt*>(&stmt)) {
        if (returnStmt->value) {
            Type valueType = analyzeExpression(*returnStmt->value);
            if (!typesCompatible(returnType, valueType)) {
                throw error("return type mismatch");
            }
        } else if (!returnType.isVoid()) {
            throw error("function must return a value");
        }
        return;
    }

    if (auto* ifStmt = dynamic_cast<IfStmt*>(&stmt)) {
        Type condType = analyzeExpression(*ifStmt->condition);
        if (!condType.isNumeric()) {
            throw error("if condition must be numeric");
        }
        analyzeStatement(*ifStmt->thenBranch, returnType);
        if (ifStmt->elseBranch) {
            analyzeStatement(*ifStmt->elseBranch, returnType);
        }
        return;
    }

    if (auto* whileStmt = dynamic_cast<WhileStmt*>(&stmt)) {
        Type condType = analyzeExpression(*whileStmt->condition);
        if (!condType.isNumeric()) {
            throw error("while condition must be numeric");
        }
        analyzeStatement(*whileStmt->body, returnType);
        return;
    }

    if (auto* forStmt = dynamic_cast<ForStmt*>(&stmt)) {
        if (forStmt->initializer) {
            analyzeStatement(*forStmt->initializer, returnType);
        }
        if (forStmt->condition) {
            Type condType = analyzeExpression(*forStmt->condition);
            if (!condType.isNumeric()) {
                throw error("for condition must be numeric");
            }
        }
        if (forStmt->increment) {
            analyzeExpression(*forStmt->increment);
        }
        analyzeStatement(*forStmt->body, returnType);
        return;
    }

    if (auto* doWhileStmt = dynamic_cast<DoWhileStmt*>(&stmt)) {
        analyzeStatement(*doWhileStmt->body, returnType);
        Type condType = analyzeExpression(*doWhileStmt->condition);
        if (!condType.isNumeric()) {
            throw error("do-while condition must be numeric");
        }
        return;
    }

    if (dynamic_cast<BreakStmt*>(&stmt) || dynamic_cast<ContinueStmt*>(&stmt)) {
        return;
    }

    if (auto* switchStmt = dynamic_cast<SwitchStmt*>(&stmt)) {
        Type condType = analyzeExpression(*switchStmt->condition);
        if (!condType.isNumeric()) {
            throw error("switch condition must be numeric");
        }
        for (auto& cb : switchStmt->cases) {
            for (auto& s : cb.statements) {
                analyzeStatement(*s, returnType);
            }
        }
        return;
    }

    if (auto* block = dynamic_cast<BlockStmt*>(&stmt)) {
        analyzeBlock(*block, returnType);
        return;
    }

    throw error("unsupported statement");
}

Type SemanticAnalyzer::analyzeExpression(Expr& expr) {
    if (dynamic_cast<IntLiteral*>(&expr)) {
        return Type::makeInt();
    }
    if (dynamic_cast<FloatLiteral*>(&expr)) {
        return Type::makeFloat();
    }
    if (dynamic_cast<CharLiteral*>(&expr)) {
        return Type::makeChar();
    }
    if (dynamic_cast<StringLiteral*>(&expr)) {
        return Type::makePointer(Type::makeChar());
    }
    if (dynamic_cast<BoolLiteral*>(&expr)) {
        return Type::makeBool();
    }

    if (auto* ternary = dynamic_cast<TernaryExpr*>(&expr)) {
        Type condType = analyzeExpression(*ternary->condition);
        if (!condType.isNumeric()) {
            throw error("ternary condition must be numeric");
        }
        Type thenType = analyzeExpression(*ternary->thenExpr);
        Type elseType = analyzeExpression(*ternary->elseExpr);
        if (!typesCompatible(thenType, elseType)) {
            throw error("ternary branch type mismatch");
        }
        return thenType;
    }

    if (auto* var = dynamic_cast<VarExpr*>(&expr)) {
        return resolveVariable(var->name);
    }

    if (auto* assign = dynamic_cast<AssignExpr*>(&expr)) {
        Type targetType = analyzeLValue(*assign->target);
        Type valueType = analyzeExpression(*assign->value);
        if (!typesCompatible(targetType, valueType)) {
            throw error("assignment type mismatch");
        }
        return targetType;
    }

    if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        Type operandType = analyzeExpression(*unary->expr);
        if (unary->op == '-') {
            if (!operandType.isNumeric()) {
                throw error("unary '-' requires numeric operand");
            }
            return Type::makeInt();
        }
        if (unary->op == '!') {
            if (!operandType.isNumeric()) {
                throw error("unary '!' requires numeric operand");
            }
            return Type::makeBool();
        }
        if (unary->op == '*') {
            if (!operandType.isPointer()) {
                throw error("dereference requires pointer operand");
            }
            return *operandType.pointee;
        }
        if (unary->op == '&') {
            return Type::makePointer(operandType);
        }
        throw error("unsupported unary operator");
    }

    if (auto* binary = dynamic_cast<BinaryExpr*>(&expr)) {
        Type leftType = analyzeExpression(*binary->left);
        Type rightType = analyzeExpression(*binary->right);
        if (binary->op == '+' && leftType.isPointer() && rightType.isInt()) {
            return leftType;
        }
        if (binary->op == '+' && leftType.isInt() && rightType.isPointer()) {
            return rightType;
        }
        if (!leftType.isNumeric() || !rightType.isNumeric()) {
            throw error("binary operator requires numeric operands");
        }
        if (binary->op == '=' || binary->op == '!' || binary->op == '<' || binary->op == '>' ||
            binary->op == 'l' || binary->op == 'g' || binary->op == '&' || binary->op == '|') {
            return Type::makeBool();
        }
        return Type::makeInt();
    }

    if (auto* index = dynamic_cast<IndexExpr*>(&expr)) {
        Type objectType = analyzeExpression(*index->object);
        Type indexType = analyzeExpression(*index->index);
        if (!indexType.isNumeric()) {
            throw error("array index must be numeric");
        }
        if (objectType.isArray()) {
            return *objectType.pointee;
        }
        if (objectType.isPointer()) {
            return *objectType.pointee;
        }
        throw error("index operator requires array or pointer");
    }

    if (auto* member = dynamic_cast<MemberExpr*>(&expr)) {
        Type objectType = analyzeExpression(*member->object);
        if (objectType.isPointer()) {
            return resolveField(*objectType.pointee, member->member);
        }
        return resolveField(objectType, member->member);
    }

    if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
        auto it = functions_.find(call->callee);
        if (it == functions_.end()) {
            throw error("undefined function '" + call->callee + "'");
        }
        const FunctionSymbol& symbol = it->second;
        if (call->args.size() != symbol.paramTypes.size()) {
            throw error("wrong number of arguments for '" + call->callee + "'");
        }
        for (size_t i = 0; i < call->args.size(); ++i) {
            Type argType = analyzeExpression(*call->args[i]);
            if (!typesCompatible(symbol.paramTypes[i], argType)) {
                throw error("argument type mismatch in call to '" + call->callee + "'");
            }
        }
        return symbol.returnType;
    }

    if (auto* call = dynamic_cast<MethodCallExpr*>(&expr)) {
        Type objectType = analyzeExpression(*call->object);
        Type baseType = objectType;
        if (baseType.isPointer()) {
            baseType = *baseType.pointee;
        }
        if (!baseType.isStruct()) {
            throw error("method call requires struct object");
        }
        std::string mangled = mangleMethod(baseType.structName, call->method);
        auto it = functions_.find(mangled);
        if (it == functions_.end()) {
            throw error("undefined method '" + call->method + "' on struct '" + baseType.structName + "'");
        }
        const FunctionSymbol& symbol = it->second;
        if (call->args.size() + 1 != symbol.paramTypes.size()) {
            throw error("wrong number of arguments for method '" + call->method + "'");
        }
        Type thisType = symbol.paramTypes[0];
        if (!typesCompatible(thisType, objectType.isPointer() ? objectType : Type::makePointer(objectType))) {
            throw error("object type mismatch in method call");
        }
        for (size_t i = 0; i < call->args.size(); ++i) {
            Type argType = analyzeExpression(*call->args[i]);
            if (!typesCompatible(symbol.paramTypes[i + 1], argType)) {
                throw error("argument type mismatch in method call");
            }
        }
        return symbol.returnType;
    }

    throw error("unsupported expression");
}

Type SemanticAnalyzer::analyzeLValue(Expr& expr) {
    if (auto* var = dynamic_cast<VarExpr*>(&expr)) {
        return resolveVariable(var->name);
    }
    if (auto* index = dynamic_cast<IndexExpr*>(&expr)) {
        Type objectType = analyzeExpression(*index->object);
        Type indexType = analyzeExpression(*index->index);
        if (!indexType.isNumeric()) {
            throw error("array index must be numeric");
        }
        if (objectType.isArray()) {
            return *objectType.pointee;
        }
        if (objectType.isPointer()) {
            return *objectType.pointee;
        }
        throw error("index assignment requires array or pointer");
    }
    if (auto* member = dynamic_cast<MemberExpr*>(&expr)) {
        Type objectType = analyzeExpression(*member->object);
        if (objectType.isPointer()) {
            return resolveField(*objectType.pointee, member->member);
        }
        return resolveField(objectType, member->member);
    }
    if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        if (unary->op == '*') {
            Type operandType = analyzeExpression(*unary->expr);
            if (!operandType.isPointer()) {
                throw error("dereference assignment requires pointer");
            }
            return *operandType.pointee;
        }
    }
    throw error("invalid assignment target");
}

void SemanticAnalyzer::declareVariable(const std::string& name, Type type) {
    if (scopes_.empty()) {
        scopes_.push_back({});
    }
    if (scopes_.back().count(name)) {
        throw error("variable '" + name + "' already declared in this scope");
    }
    scopes_.back()[name] = type;
}

Type SemanticAnalyzer::resolveVariable(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    throw error("undefined variable '" + name + "'");
}

Type SemanticAnalyzer::resolveField(const Type& objectType, const std::string& member) const {
    if (!objectType.isStruct() || !objectType.structDef) {
        throw error("member access requires struct type");
    }
    for (const auto& field : objectType.structDef->fields) {
        if (field.name == member) {
            return field.type;
        }
    }
    throw error("struct '" + objectType.structName + "' has no member '" + member + "'");
}

bool SemanticAnalyzer::typesCompatible(const Type& target, const Type& value) const {
    if (target.equals(value)) {
        return true;
    }
    if (target.isInt() && value.isBool()) {
        return true;
    }
    if (target.isBool() && value.isInt()) {
        return true;
    }
    return false;
}

void SemanticAnalyzer::beginScope() { scopes_.push_back({}); }
void SemanticAnalyzer::endScope() { scopes_.pop_back(); }

std::string SemanticAnalyzer::mangleMethod(const std::string& structName,
                                           const std::string& method) const {
    return structName + "_" + method;
}

SemanticError SemanticAnalyzer::error(const std::string& message) const {
    return SemanticError(message);
}
