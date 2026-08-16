cmake_minimum_required(VERSION 3.16)
project(rosie VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rosie
    src/main.cpp
    src/token.cpp
    src/lexer.cpp
    src/type.cpp
    src/ast.cpp
    src/parser.cpp
    src/semantic.cpp
    src/ir.cpp
    src/optimizer.cpp
    src/codegen.cpp
)

target_include_directories(rosie PRIVATE include)

if(MSVC)
    target_compile_options(rosie PRIVATE /W4)
else()
    target_compile_options(rosie PRIVATE -Wall -Wextra -pedantic)
endif()
