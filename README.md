# Rosie C++ Compiler (`rosie`)

> A multi-pass C++ compiler written in C++17. Lexes, parses, type-checks, optimizes via Three-Address Code (TAC) Intermediate Representation, generates native x86-64 assembly, and links executable binaries.

---

## 🏛 Architecture & Pipeline Design

Rosie implements an industrial-grade, decoupled compiler pipeline matching the architecture of GCC and LLVM:

```
Source Code → Preprocessor → Lexer → Parser → AST → Type Checker → TAC IR Gen → Optimizer (-O1/-O2) → x86-64 CodeGen → Linker → Executable
```

| Phase | Description |
|-------|-------------|
| **Preprocessor** | Integrated C preprocessor pass for `#include` header inclusion and `#define` macro expansion. |
| **Lexer** | Scans C++ keywords, identifiers, string/char/float literals, multiline comments (`/* ... */`), and operators with line/column position tracking. |
| **Diagnostics Engine** | Colorized diagnostic formatting displaying source line context snippets and precise column caret markers (`^~~~`). |
| **Parser** | Hand-crafted precedence-climbing recursive descent parser building an Abstract Syntax Tree (AST). |
| **AST Visualizer** | Indented ASCII visual tree printer (`--dump-ast`). |
| **Type Checker** | Scoped symbol tables, static type checking, struct/class layout computation, and method mangling. |
| **Intermediate Representation** | Decoupled Three-Address Code (TAC) linear IR engine (`--emit-ir`). |
| **Optimizer** | Multi-pass IR optimizer (`-O0`, `-O1`, `-O2`): Constant Folding, Dead Code Elimination, Algebraic Simplifications. |
| **Code Generator** | Emits Win64 / System V AMD64 ABI compliant x86-64 Intel assembly syntax. |
| **Linker** | Integrates with GNU assembler (`as`) & linker driver (`g++`) to produce native executables. |

---

## 🚀 Key Features & Language Support Matrix

| Category | Features Supported |
|----------|-------------------|
| **Data Types** | `int`, `bool`, `char`, `float`, `double`, `void`, Pointers (`T*`), Arrays (`T[N]`), `struct`, `class`, References (`T&`), String Literals (`"..."`). |
| **Control Flow** | `if` / `else`, `while`, `for`, `do-while`, `switch` / `case` / `default`, `break`, `continue`, `return`. |
| **Operators** | Arithmetic (`+`, `-`, `*`, `/`, `%`), Comparisons (`==`, `!=`, `<`, `>`, `<=`, `>=`), Logical (`&&`, `||`, `!`), Bitwise (`&`, `|`, `^`, `~`, `<<`, `>>`), Ternary (`? :`), Increment/Decrement (`++`, `--`), Compound Assignments (`+=`, `-=`, `*=`, `/=`, `%=`), Member Access (`.`, `->`), Array Indexing (`[]`). |
| **Object-Oriented Programming** | Classes, Structs, Member fields, Member methods (`this` pointer passing in `RCX`), Access labels (`public:`, `private:`), Method name mangling (`Class_method`). |
| **Optimizations** | **Constant Folding**: Compile-time constant evaluation (`10 + 20 * 3` → `70`).<br>**Dead Code Elimination (DCE)**: Prunes instructions after `return` and dead jump branches.<br>**Algebraic Simplifications**: Simplifies `x + 0`, `x * 1`, `x * 0`. |

---

## 🛠 Build Instructions

Requires a **C++17 compiler** (GCC, Clang, or MSVC) and optionally **CMake 3.16+**.

### Building with MSYS2 GCC
```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Iinclude -O2 src/main.cpp src/token.cpp src/lexer.cpp src/type.cpp src/ast.cpp src/parser.cpp src/semantic.cpp src/ir.cpp src/optimizer.cpp src/codegen.cpp -o rosie.exe
```

### Building with CMake
```bash
cmake -B build
cmake --build build
```
The compiled binary will be at `rosie.exe` (or `build/Debug/rosie.exe`).

---

## 💻 Usage & CLI Options

```bash
# Basic compilation to binary
.\rosie.exe examples\rosie_capstone_demo.cpp -o capstone.exe
.\capstone.exe
echo $LASTEXITCODE  # Returns 261

# Dump visual Abstract Syntax Tree (AST)
.\rosie.exe examples\oop_class.cpp --dump-ast

# Emit optimized Three-Address Code (TAC) IR
.\rosie.exe examples\opt_test.cpp -O2 --emit-ir

# Emit x86-64 assembly (.s) without linking
.\rosie.exe examples\rosie_capstone_demo.cpp --emit-asm

# Verbose compilation pipeline stages
.\rosie.exe examples\rosie_capstone_demo.cpp -v -o demo.exe
```

### Command Flags

| Flag | Description |
|------|-------------|
| `-o <file>` | Output executable binary path (default: `a.exe` / `a.out`) |
| `-O0`, `-O1`, `-O2` | Set optimization level (0: disabled, 1: standard, 2: aggressive) |
| `--emit-asm`, `-S` | Emit x86-64 assembly file (`.s`) and skip linking |
| `--emit-ir` | Emit Three-Address Code (TAC) IR stream and exit |
| `--dump-ast` | Render visual AST tree hierarchy and exit |
| `--dump-tokens` | Print lexer token stream table and exit |
| `-v`, `--verbose` | Print detailed timing and pipeline execution stages |
| `--version` | Display Rosie compiler version banner |
| `-h`, `--help` | Display usage manual |

---

## 🎓 Evaluation & Defense Presentation Guide

When presenting Rosie to your capstone evaluation panel or professors, follow this 4-step demonstration:

1. **Project Pitch**:
   > *"Rosie: A Multi-Pass C++ Compiler with Three-Address Code (TAC) Optimization and Native x86-64 Assembly Generation"*

2. **Step 1 — Visual AST Tree (`--dump-ast`)**:
   Run `.\rosie.exe examples\oop_class.cpp --dump-ast` to demonstrate parsing of classes, methods, and fields into an AST tree hierarchy:
   ```
   --- Rosie Abstract Syntax Tree ---
   Program
     ├── Struct/Class: Vector2D (2 fields, 3 methods)
     │   ├── Field: int x
     │   ├── Field: int y
     │   └── Method: int getX()
     │   └── Method: int getY()
     │   └── Method: int dot()
     └── Function: int main()
   ```

3. **Step 2 — Optimization Pipeline (`--emit-ir` `-O2`)**:
   Run `.\rosie.exe examples\opt_test.cpp -O2 --emit-ir` to showcase compile-time constant evaluation and dead code elimination:
   ```asm
   ; --- Rosie Three-Address Code (TAC) IR ---
   function constantTest:
       t0 = 60
       t1 = 10 + t0
       a = t1
       t2 = a
       t3 = t2
       b = t3
       return b
   ```

4. **Step 3 — Native Binary Execution**:
   Run `.\rosie.exe examples\rosie_capstone_demo.cpp -o capstone.exe`, execute `.\capstone.exe`, and check the exit code (`261`), demonstrating native execution of loops, switch-case, ternary expressions, and function calls.

---

## 📄 License

MIT License
