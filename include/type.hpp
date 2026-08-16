#pragma once

#include <memory>
#include <string>

struct StructDecl;

enum class TypeKind { Int, Bool, Char, Float, Double, Void, Pointer, Reference, Array, Struct };

struct Type {
    TypeKind kind = TypeKind::Int;
    std::shared_ptr<Type> pointee;
    int arraySize = 0;
    std::string structName;
    const StructDecl* structDef = nullptr;

    static Type makeInt();
    static Type makeBool();
    static Type makeChar();
    static Type makeFloat();
    static Type makeDouble();
    static Type makeVoid();
    static Type makePointer(Type base);
    static Type makeReference(Type base);
    static Type makeArray(Type element, int size);
    static Type makeStruct(const std::string& name, const StructDecl* def);

    bool isInt() const { return kind == TypeKind::Int; }
    bool isBool() const { return kind == TypeKind::Bool; }
    bool isChar() const { return kind == TypeKind::Char; }
    bool isFloat() const { return kind == TypeKind::Float; }
    bool isDouble() const { return kind == TypeKind::Double; }
    bool isVoid() const { return kind == TypeKind::Void; }
    bool isPointer() const { return kind == TypeKind::Pointer; }
    bool isReference() const { return kind == TypeKind::Reference; }
    bool isArray() const { return kind == TypeKind::Array; }
    bool isStruct() const { return kind == TypeKind::Struct; }
    bool isNumeric() const { return isInt() || isBool() || isChar() || isFloat() || isDouble(); }

    size_t sizeBytes() const;
    Type decay() const;
    bool equals(const Type& other) const;
    std::string toString() const;
};
