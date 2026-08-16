#include "codegen.hpp"

#include <sstream>

void CodeGenerator::emit(const Program& program, std::ostream& out) {
    out_ = &out;

    *out_ << ".intel_syntax noprefix\n";
    *out_ << ".text\n";

    auto emitSingleFn = [&](const FunctionDecl& fn, const std::string& name) {
        locals_.clear();
        localScopes_.clear();
        stackOffset_ = 0;
        labelCounter_ = 0;

        *out_ << ".globl " << name << "\n";
        *out_ << name << ":\n";

        emitFunctionPrologue(fn);

        beginScope();
        for (const auto& param : fn.params) {
            if (lookupLocal(param.second) < 0) {
                allocateLocal(param.second);
            }
        }

        if (fn.body) {
            for (const auto& stmt : fn.body->statements) {
                emitStatement(*stmt, name);
            }
        }

        if (name == "main") {
            *out_ << "    xor eax, eax\n";
        }
        *out_ << ".L_" << name << "_end:\n";
        emitFunctionEpilogue();
        endScope();
    };

    for (const auto& fn : program.functions) {
        emitSingleFn(*fn, fn->name);
    }

    for (const auto& s : program.structs) {
        for (const auto& m : s->methods) {
            std::string mangled = s->name + "_" + m->name;
            emitSingleFn(*m, mangled);
        }
    }
}

std::string CodeGenerator::freshLabel(const std::string& prefix) {
    return ".L_" + prefix + "_" + std::to_string(labelCounter_++);
}

void CodeGenerator::emitFunctionPrologue(const FunctionDecl& fn) {
    *out_ << "    push rbp\n";
    *out_ << "    mov rbp, rsp\n";
    *out_ << "    push rbx\n";
    *out_ << "    push r12\n";
    *out_ << "    push r13\n";
    *out_ << "    sub rsp, 128\n";

    static const char* argRegs[] = {"rcx", "rdx", "r8", "r9"};
    for (size_t i = 0; i < fn.params.size() && i < 4; ++i) {
        int offset = allocateLocal(fn.params[i].second);
        *out_ << "    mov [rbp-" << offset << "], " << argRegs[i] << "\n";
    }
}

void CodeGenerator::emitFunctionEpilogue() {
    *out_ << "    add rsp, 128\n";
    *out_ << "    pop r13\n";
    *out_ << "    pop r12\n";
    *out_ << "    pop rbx\n";
    *out_ << "    pop rbp\n";
    *out_ << "    ret\n";
}

void CodeGenerator::beginScope() {
    localScopes_.push_back({});
}

void CodeGenerator::endScope() {
    localScopes_.pop_back();
}

int CodeGenerator::allocateLocal(const std::string& name) {
    stackOffset_ += 8;
    locals_[name] = stackOffset_;
    if (!localScopes_.empty()) {
        localScopes_.back()[name] = stackOffset_;
    }
    return stackOffset_;
}

int CodeGenerator::lookupLocal(const std::string& name) const {
    for (auto it = localScopes_.rbegin(); it != localScopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    auto found = locals_.find(name);
    if (found != locals_.end()) {
        return found->second;
    }
    return -1;
}

void CodeGenerator::pushRegister(const char* reg) {
    *out_ << "    push " << reg << "\n";
}

void CodeGenerator::popToRegister(const char* reg) {
    *out_ << "    pop " << reg << "\n";
}

void CodeGenerator::emitExpression(const Expr& expr) {
    if (auto* literal = dynamic_cast<const IntLiteral*>(&expr)) {
        *out_ << "    mov rax, " << literal->value << "\n";
        pushRegister("rax");
        return;
    }

    if (auto* var = dynamic_cast<const VarExpr*>(&expr)) {
        int offset = lookupLocal(var->name);
        *out_ << "    mov rax, [rbp-" << offset << "]\n";
        pushRegister("rax");
        return;
    }

    if (auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        emitExpression(*assign->value);
        popToRegister("rax");
        if (auto* varTarget = dynamic_cast<const VarExpr*>(assign->target.get())) {
            int offset = lookupLocal(varTarget->name);
            *out_ << "    mov [rbp-" << offset << "], rax\n";
        }
        pushRegister("rax");
        return;
    }

    if (auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        emitExpression(*unary->expr);
        popToRegister("rax");
        if (unary->op == '-') {
            *out_ << "    neg rax\n";
        } else if (unary->op == '!') {
            *out_ << "    test rax, rax\n";
            *out_ << "    sete al\n";
            *out_ << "    movzx rax, al\n";
        }
        pushRegister("rax");
        return;
    }

    if (auto* flit = dynamic_cast<const FloatLiteral*>(&expr)) {
        *out_ << "    mov rax, " << static_cast<long long>(flit->value) << "\n";
        pushRegister("rax");
        return;
    }

    if (auto* clit = dynamic_cast<const CharLiteral*>(&expr)) {
        *out_ << "    mov rax, " << static_cast<int>(clit->value) << "\n";
        pushRegister("rax");
        return;
    }

    if (auto* tern = dynamic_cast<const TernaryExpr*>(&expr)) {
        std::string elseLbl = freshLabel("tern_else");
        std::string endLbl = freshLabel("tern_end");
        emitExpression(*tern->condition);
        popToRegister("rax");
        *out_ << "    test rax, rax\n";
        *out_ << "    jz " << elseLbl << "\n";
        emitExpression(*tern->thenExpr);
        *out_ << "    jmp " << endLbl << "\n";
        *out_ << elseLbl << ":\n";
        emitExpression(*tern->elseExpr);
        *out_ << endLbl << ":\n";
        return;
    }

    if (auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        emitExpression(*binary->left);
        emitExpression(*binary->right);
        popToRegister("rbx");
        popToRegister("rax");

        switch (binary->op) {
            case '+': *out_ << "    add rax, rbx\n"; break;
            case '-': *out_ << "    sub rax, rbx\n"; break;
            case '*': *out_ << "    imul rax, rbx\n"; break;
            case '/': *out_ << "    cqo\n    idiv rbx\n"; break;
            case '%': *out_ << "    cqo\n    idiv rbx\n    mov rax, rdx\n"; break;
            case '<': *out_ << "    cmp rax, rbx\n    setl al\n    movzx rax, al\n"; break;
            case '>': *out_ << "    cmp rax, rbx\n    setg al\n    movzx rax, al\n"; break;
            case 'l': *out_ << "    cmp rax, rbx\n    setle al\n    movzx rax, al\n"; break;
            case 'g': *out_ << "    cmp rax, rbx\n    setge al\n    movzx rax, al\n"; break;
            case '=': *out_ << "    cmp rax, rbx\n    sete al\n    movzx rax, al\n"; break;
            case '!': *out_ << "    cmp rax, rbx\n    setne al\n    movzx rax, al\n"; break;
            case '&': *out_ << "    and rax, rbx\n"; break;
            case '|': *out_ << "    or rax, rbx\n"; break;
            case '^': *out_ << "    xor rax, rbx\n"; break;
            default: break;
        }
        pushRegister("rax");
        return;
    }

    if (auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        static const char* argRegs[] = {"rcx", "rdx", "r8", "r9"};
        size_t argCount = call->args.size();
        if (argCount > 4) argCount = 4;

        for (size_t i = argCount; i > 0; --i) {
            emitExpression(*call->args[i - 1]);
            popToRegister(argRegs[i - 1]);
        }

        *out_ << "    sub rsp, 32\n";
        *out_ << "    call " << call->callee << "\n";
        *out_ << "    add rsp, 32\n";
        pushRegister("rax");
        return;
    }
}

void CodeGenerator::emitStatement(const Stmt& stmt, const std::string& currentFunction) {
    if (auto* varDecl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        allocateLocal(varDecl->name);
        if (varDecl->initializer) {
            emitExpression(*varDecl->initializer);
            popToRegister("rax");
            int offset = lookupLocal(varDecl->name);
            *out_ << "    mov [rbp-" << offset << "], rax\n";
        } else {
            int offset = lookupLocal(varDecl->name);
            *out_ << "    mov qword ptr [rbp-" << offset << "], 0\n";
        }
        return;
    }

    if (auto* exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        emitExpression(*exprStmt->expr);
        popToRegister("rax");
        return;
    }

    if (auto* returnStmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (returnStmt->value) {
            emitExpression(*returnStmt->value);
            popToRegister("rax");
        }
        *out_ << "    jmp .L_" << currentFunction << "_end\n";
        return;
    }

    if (auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        std::string elseLabel = freshLabel("else");
        std::string endLabel = freshLabel("endif");

        emitExpression(*ifStmt->condition);
        popToRegister("rax");
        *out_ << "    test rax, rax\n";
        *out_ << "    jz " << elseLabel << "\n";
        emitStatement(*ifStmt->thenBranch, currentFunction);
        *out_ << "    jmp " << endLabel << "\n";
        *out_ << elseLabel << ":\n";
        if (ifStmt->elseBranch) {
            emitStatement(*ifStmt->elseBranch, currentFunction);
        }
        *out_ << endLabel << ":\n";
        return;
    }

    if (dynamic_cast<const BreakStmt*>(&stmt)) {
        if (!breakTargets_.empty()) {
            *out_ << "    jmp " << breakTargets_.back() << "\n";
        }
        return;
    }

    if (dynamic_cast<const ContinueStmt*>(&stmt)) {
        if (!continueTargets_.empty()) {
            *out_ << "    jmp " << continueTargets_.back() << "\n";
        }
        return;
    }

    if (auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        std::string startLabel = freshLabel("while");
        std::string endLabel = freshLabel("wend");

        breakTargets_.push_back(endLabel);
        continueTargets_.push_back(startLabel);

        *out_ << startLabel << ":\n";
        emitExpression(*whileStmt->condition);
        popToRegister("rax");
        *out_ << "    test rax, rax\n";
        *out_ << "    jz " << endLabel << "\n";
        emitStatement(*whileStmt->body, currentFunction);
        *out_ << "    jmp " << startLabel << "\n";
        *out_ << endLabel << ":\n";

        breakTargets_.pop_back();
        continueTargets_.pop_back();
        return;
    }

    if (auto* doWhileStmt = dynamic_cast<const DoWhileStmt*>(&stmt)) {
        std::string startLabel = freshLabel("dowhile");
        std::string endLabel = freshLabel("dend");

        breakTargets_.push_back(endLabel);
        continueTargets_.push_back(startLabel);

        *out_ << startLabel << ":\n";
        emitStatement(*doWhileStmt->body, currentFunction);
        emitExpression(*doWhileStmt->condition);
        popToRegister("rax");
        *out_ << "    test rax, rax\n";
        *out_ << "    jnz " << startLabel << "\n";
        *out_ << endLabel << ":\n";

        breakTargets_.pop_back();
        continueTargets_.pop_back();
        return;
    }

    if (auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        std::string startLabel = freshLabel("for");
        std::string endLabel = freshLabel("fend");

        breakTargets_.push_back(endLabel);
        continueTargets_.push_back(startLabel);

        if (forStmt->initializer) emitStatement(*forStmt->initializer, currentFunction);
        *out_ << startLabel << ":\n";
        if (forStmt->condition) {
            emitExpression(*forStmt->condition);
            popToRegister("rax");
            *out_ << "    test rax, rax\n";
            *out_ << "    jz " << endLabel << "\n";
        }
        if (forStmt->body) emitStatement(*forStmt->body, currentFunction);
        if (forStmt->increment) {
            emitExpression(*forStmt->increment);
            popToRegister("rax");
        }
        *out_ << "    jmp " << startLabel << "\n";
        *out_ << endLabel << ":\n";

        breakTargets_.pop_back();
        continueTargets_.pop_back();
        return;
    }

    if (auto* switchStmt = dynamic_cast<const SwitchStmt*>(&stmt)) {
        std::string endLabel = freshLabel("swend");
        breakTargets_.push_back(endLabel);

        emitExpression(*switchStmt->condition);
        popToRegister("rax");

        std::vector<std::string> caseLabels;
        for (const auto& cb : switchStmt->cases) {
            caseLabels.push_back(freshLabel(cb.isDefault ? "default" : "case"));
        }

        for (size_t i = 0; i < switchStmt->cases.size(); ++i) {
            const auto& cb = switchStmt->cases[i];
            const auto& caseLbl = caseLabels[i];
            if (!cb.isDefault) {
                *out_ << "    cmp rax, " << cb.value << "\n";
                *out_ << "    je " << caseLbl << "\n";
            } else {
                *out_ << "    jmp " << caseLbl << "\n";
            }
        }
        *out_ << "    jmp " << endLabel << "\n";

        for (size_t i = 0; i < switchStmt->cases.size(); ++i) {
            const auto& cb = switchStmt->cases[i];
            const auto& caseLbl = caseLabels[i];
            *out_ << caseLbl << ":\n";
            for (const auto& s : cb.statements) {
                emitStatement(*s, currentFunction);
            }
        }
        *out_ << endLabel << ":\n";
        breakTargets_.pop_back();
        return;
    }

    if (auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
        beginScope();
        for (const auto& inner : block->statements) {
            emitStatement(*inner, currentFunction);
        }
        endScope();
        return;
    }
}
