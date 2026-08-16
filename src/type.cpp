#include "type.hpp"

#include "ast.hpp"

Type Type::makeInt() { return Type{TypeKind::Int}; }
Type Type::makeBool() { return Type{TypeKind::Bool}; }
Type Type::makeChar() { return Type{TypeKind::Char}; }
Type Type::makeFloat() { return Type{TypeKind::Float}; }
Type Type::makeDouble() { return Type{TypeKind::Double}; }
Type Type::makeVoid() { return Type{TypeKind::Void}; }

Type Type::makePointer(Type base) {
    Type t;
    t.kind = TypeKind::Pointer;
    t.pointee = std::make_shared<Type>(base);
    return t;
}

Type Type::makeReference(Type base) {
    Type t;
    t.kind = TypeKind::Reference;
    t.pointee = std::make_shared<Type>(base);
    return t;
}

Type Type::makeArray(Type element, int size) {
    Type t;
    t.kind = TypeKind::Array;
    t.arraySize = size;
    t.pointee = std::make_shared<Type>(element);
    return t;
}

Type Type::makeStruct(const std::string& name, const StructDecl* def) {
    Type t;
    t.kind = TypeKind::Struct;
    t.structName = name;
    t.structDef = def;
    return t;
}

size_t Type::sizeBytes() const {
    switch (kind) {
        case TypeKind::Int:
        case TypeKind::Bool:
        case TypeKind::Char:
        case TypeKind::Float:
        case TypeKind::Double:
        case TypeKind::Pointer:
        case TypeKind::Reference:
            return 8;
        case TypeKind::Void:
            return 0;
        case TypeKind::Array:
            return pointee ? pointee->sizeBytes() * static_cast<size_t>(arraySize) : 0;
        case TypeKind::Struct:
            return structDef ? structDef->sizeBytes : 0;
    }
    return 8;
}

Type Type::decay() const {
    if (isArray() && pointee) {
        return makePointer(*pointee);
    }
    return *this;
}

bool Type::equals(const Type& other) const {
    if (kind != other.kind) {
        return false;
    }
    switch (kind) {
        case TypeKind::Int:
        case TypeKind::Bool:
        case TypeKind::Char:
        case TypeKind::Float:
        case TypeKind::Double:
        case TypeKind::Void:
            return true;
        case TypeKind::Pointer:
        case TypeKind::Reference:
            return pointee && other.pointee && pointee->equals(*other.pointee);
        case TypeKind::Array:
            return arraySize == other.arraySize && pointee && other.pointee &&
                   pointee->equals(*other.pointee);
        case TypeKind::Struct:
            return structName == other.structName;
    }
    return false;
}

std::string Type::toString() const {
    switch (kind) {
        case TypeKind::Int: return "int";
        case TypeKind::Bool: return "bool";
        case TypeKind::Char: return "char";
        case TypeKind::Float: return "float";
        case TypeKind::Double: return "double";
        case TypeKind::Void: return "void";
        case TypeKind::Pointer: return pointee ? pointee->toString() + "*" : "void*";
        case TypeKind::Reference: return pointee ? pointee->toString() + "&" : "void&";
        case TypeKind::Array:
            return pointee ? pointee->toString() + "[" + std::to_string(arraySize) + "]" : "array";
        case TypeKind::Struct: return structName;
    }
    return "?";
}
