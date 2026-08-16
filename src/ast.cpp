#include "ast.hpp"
#include <iostream>

void ASTPrinter::print(const Program& program, std::ostream& out) {
    out << "Program\n";
    for (const auto& s : program.structs) {
        out << "  ├── Struct/Class: " << s->name << " (" << s->fields.size() << " fields, "
            << s->methods.size() << " methods)\n";
        for (const auto& f : s->fields) {
            out << "  │   ├── Field: " << f.type.toString() << " " << f.name << "\n";
        }
        for (const auto& m : s->methods) {
            out << "  │   └── Method: " << m->returnType.toString() << " " << m->name << "()\n";
        }
    }
    for (const auto& ns : program.namespaces) {
        out << "  ├── Namespace: " << ns->name << "\n";
        for (const auto& fn : ns->functions) {
            out << "  │   └── Function: " << fn->returnType.toString() << " " << fn->name << "()\n";
        }
    }
    for (const auto& fn : program.functions) {
        out << "  └── Function: " << fn->returnType.toString() << " " << fn->name << "(";
        for (size_t i = 0; i < fn->params.size(); ++i) {
            if (i > 0) out << ", ";
            out << fn->params[i].first.toString() << " " << fn->params[i].second;
        }
        out << ")\n";
    }
}
