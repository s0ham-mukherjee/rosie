#include "ir.hpp"
#include <sstream>

namespace ir {

std::string Instruction::toString() const {
    std::ostringstream ss;
    switch (op) {
        case OpCode::ASSIGN: ss << result << " = " << arg1; break;
        case OpCode::ADD: ss << result << " = " << arg1 << " + " << arg2; break;
        case OpCode::SUB: ss << result << " = " << arg1 << " - " << arg2; break;
        case OpCode::MUL: ss << result << " = " << arg1 << " * " << arg2; break;
        case OpCode::DIV: ss << result << " = " << arg1 << " / " << arg2; break;
        case OpCode::MOD: ss << result << " = " << arg1 << " % " << arg2; break;
        case OpCode::EQ: ss << result << " = " << arg1 << " == " << arg2; break;
        case OpCode::NEQ: ss << result << " = " << arg1 << " != " << arg2; break;
        case OpCode::LT: ss << result << " = " << arg1 << " < " << arg2; break;
        case OpCode::GT: ss << result << " = " << arg1 << " > " << arg2; break;
        case OpCode::LE: ss << result << " = " << arg1 << " <= " << arg2; break;
        case OpCode::GE: ss << result << " = " << arg1 << " >= " << arg2; break;
        case OpCode::AND: ss << result << " = " << arg1 << " && " << arg2; break;
        case OpCode::OR: ss << result << " = " << arg1 << " || " << arg2; break;
        case OpCode::NOT: ss << result << " = !" << arg1; break;
        case OpCode::BIT_AND: ss << result << " = " << arg1 << " & " << arg2; break;
        case OpCode::BIT_OR: ss << result << " = " << arg1 << " | " << arg2; break;
        case OpCode::BIT_XOR: ss << result << " = " << arg1 << " ^ " << arg2; break;
        case OpCode::BIT_NOT: ss << result << " = ~" << arg1; break;
        case OpCode::SHL: ss << result << " = " << arg1 << " << " << arg2; break;
        case OpCode::SHR: ss << result << " = " << arg1 << " >> " << arg2; break;
        case OpCode::LABEL: ss << "label " << result << ":"; break;
        case OpCode::JUMP: ss << "goto " << result; break;
        case OpCode::JUMP_IF_ZERO: ss << "ifz " << arg1 << " goto " << result; break;
        case OpCode::JUMP_IF_NOT_ZERO: ss << "ifnz " << arg1 << " goto " << result; break;
        case OpCode::CALL: ss << result << " = call " << arg1 << ", args=" << arg2; break;
        case OpCode::PARAM: ss << "param " << arg1; break;
        case OpCode::RETURN: ss << "return " << arg1; break;
        case OpCode::LOAD_PTR: ss << result << " = *" << arg1; break;
        case OpCode::STORE_PTR: ss << "*" << result << " = " << arg1; break;
        case OpCode::LOAD_MEMBER: ss << result << " = " << arg1 << "." << arg2; break;
        case OpCode::STORE_MEMBER: ss << arg1 << "." << arg2 << " = " << result; break;
    }
    return ss.str();
}

void IRProgram::print(std::ostream& out) const {
    out << "; --- Rosie Three-Address Code (TAC) IR ---\n\n";
    for (const auto& fn : functions) {
        out << "function " << fn.name << ":\n";
        for (const auto& inst : fn.instructions) {
            if (inst.op == OpCode::LABEL) {
                out << "  " << inst.toString() << "\n";
            } else {
                out << "    " << inst.toString() << "\n";
            }
        }
        out << "\n";
    }
}

std::string IRGenerator::newTemp() {
    return "t" + std::to_string(tempCounter_++);
}

std::string IRGenerator::newLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(labelCounter_++);
}

IRProgram IRGenerator::generate(const Program& program) {
    irProg_.functions.clear();
    for (const auto& fn : program.functions) {
        genFunction(*fn);
    }
    for (const auto& s : program.structs) {
        for (const auto& m : s->methods) {
            genFunction(*m);
        }
    }
    return irProg_;
}

void IRGenerator::genFunction(const FunctionDecl& fn) {
    currentFn_ = IRFunction{};
    currentFn_.name = fn.name;
    if (fn.body) {
        genStatement(*fn.body);
    }
    irProg_.functions.push_back(currentFn_);
}

void IRGenerator::genStatement(const Stmt& stmt) {
    if (auto varDecl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        if (varDecl->initializer) {
            std::string val = genExpr(*varDecl->initializer);
            currentFn_.instructions.push_back({OpCode::ASSIGN, varDecl->name, val, ""});
        }
    } else if (auto exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        if (exprStmt->expr) {
            genExpr(*exprStmt->expr);
        }
    } else if (auto retStmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
        std::string val = retStmt->value ? genExpr(*retStmt->value) : "";
        currentFn_.instructions.push_back({OpCode::RETURN, "", val, ""});
    } else if (auto block = dynamic_cast<const BlockStmt*>(&stmt)) {
        for (const auto& s : block->statements) {
            genStatement(*s);
        }
    } else if (auto ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        std::string cond = genExpr(*ifStmt->condition);
        std::string elseLabel = newLabel("else");
        std::string endLabel = newLabel("endif");
        currentFn_.instructions.push_back({OpCode::JUMP_IF_ZERO, elseLabel, cond, ""});
        if (ifStmt->thenBranch) genStatement(*ifStmt->thenBranch);
        currentFn_.instructions.push_back({OpCode::JUMP, endLabel, "", ""});
        currentFn_.instructions.push_back({OpCode::LABEL, elseLabel, "", ""});
        if (ifStmt->elseBranch) genStatement(*ifStmt->elseBranch);
        currentFn_.instructions.push_back({OpCode::LABEL, endLabel, "", ""});
    } else if (auto whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        std::string startLabel = newLabel("while_start");
        std::string endLabel = newLabel("while_end");
        currentFn_.instructions.push_back({OpCode::LABEL, startLabel, "", ""});
        std::string cond = genExpr(*whileStmt->condition);
        currentFn_.instructions.push_back({OpCode::JUMP_IF_ZERO, endLabel, cond, ""});
        if (whileStmt->body) genStatement(*whileStmt->body);
        currentFn_.instructions.push_back({OpCode::JUMP, startLabel, "", ""});
        currentFn_.instructions.push_back({OpCode::LABEL, endLabel, "", ""});
    } else if (auto forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        if (forStmt->initializer) genStatement(*forStmt->initializer);
        std::string startLabel = newLabel("for_start");
        std::string endLabel = newLabel("for_end");
        currentFn_.instructions.push_back({OpCode::LABEL, startLabel, "", ""});
        if (forStmt->condition) {
            std::string cond = genExpr(*forStmt->condition);
            currentFn_.instructions.push_back({OpCode::JUMP_IF_ZERO, endLabel, cond, ""});
        }
        if (forStmt->body) genStatement(*forStmt->body);
        if (forStmt->increment) genExpr(*forStmt->increment);
        currentFn_.instructions.push_back({OpCode::JUMP, startLabel, "", ""});
        currentFn_.instructions.push_back({OpCode::LABEL, endLabel, "", ""});
    } else if (auto breakStmt = dynamic_cast<const BreakStmt*>(&stmt)) {
        currentFn_.instructions.push_back({OpCode::JUMP, "break_target", "", ""});
    } else if (auto contStmt = dynamic_cast<const ContinueStmt*>(&stmt)) {
        currentFn_.instructions.push_back({OpCode::JUMP, "continue_target", "", ""});
    }
}

std::string IRGenerator::genExpr(const Expr& expr) {
    if (auto lit = dynamic_cast<const IntLiteral*>(&expr)) {
        return std::to_string(lit->value);
    } else if (auto flit = dynamic_cast<const FloatLiteral*>(&expr)) {
        return std::to_string(flit->value);
    } else if (auto clit = dynamic_cast<const CharLiteral*>(&expr)) {
        return std::to_string(static_cast<int>(clit->value));
    } else if (auto blit = dynamic_cast<const BoolLiteral*>(&expr)) {
        return blit->value ? "1" : "0";
    } else if (auto var = dynamic_cast<const VarExpr*>(&expr)) {
        return var->name;
    } else if (auto bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        std::string l = genExpr(*bin->left);
        std::string r = genExpr(*bin->right);
        std::string t = newTemp();
        OpCode op = OpCode::ADD;
        switch (bin->op) {
            case '+': op = OpCode::ADD; break;
            case '-': op = OpCode::SUB; break;
            case '*': op = OpCode::MUL; break;
            case '/': op = OpCode::DIV; break;
            case '%': op = OpCode::MOD; break;
            case '&': op = OpCode::BIT_AND; break;
            case '|': op = OpCode::BIT_OR; break;
            case '^': op = OpCode::BIT_XOR; break;
        }
        currentFn_.instructions.push_back({op, t, l, r});
        return t;
    } else if (auto assign = dynamic_cast<const AssignExpr*>(&expr)) {
        std::string rhs = genExpr(*assign->value);
        if (auto varTarget = dynamic_cast<const VarExpr*>(assign->target.get())) {
            currentFn_.instructions.push_back({OpCode::ASSIGN, varTarget->name, rhs, ""});
            return varTarget->name;
        }
        return rhs;
    } else if (auto call = dynamic_cast<const CallExpr*>(&expr)) {
        std::vector<std::string> args;
        for (const auto& a : call->args) {
            args.push_back(genExpr(*a));
        }
        for (const auto& a : args) {
            currentFn_.instructions.push_back({OpCode::PARAM, "", a, ""});
        }
        std::string t = newTemp();
        currentFn_.instructions.push_back({OpCode::CALL, t, call->callee, std::to_string(args.size())});
        return t;
    }
    return "0";
}

} // namespace ir
