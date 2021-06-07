# hscpp: Hot reload C++ code at runtime
hotswap-cpp (hscpp) is a library that allows C++ to be reloaded at runtime. This can greatly decrease iteration time where small changes are common, such as during game scripting or UI development.

## Documentation

[The documentation can be read here.](./docs/README.md)

## Getting started

A good way to get started is to review the following demos in order:

- [simple-demo](./examples/simple-demo)
- [memory-allocation-demo](./examples/memory-allocation-demo)
- [runtime-require-demo](./examples/runtime-require-demo)
- [imgui-demo](./examples/imgui-demo)
- [dependent-compilation-demo](./examples/dependent-compilation-demo)

These demos build off of one and other to demonstrate core hscpp features.

## How it works
hscpp was inspired by and uses the same base approach as [Runtime Compiled C++](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus).

When a program that links hscpp is running, the program's source directories will be monitored for changes. When a file is modified, hscpp will recompile it into a dynamic library, and link the newly created library into the running program.

If this new library contains classes registered with hscpp via the `HSCPP_TRACK` macro, old instances of those classes are deleted and replaced by new instances running the new code.

## Features
- Cross-platform. Works on Windows, macOS, and Linux.
- Multiple compiler support out of the box:
    - On Windows: MSVC and clang-cl
    - On macOS: clang and g++ *(note: AppleClang not currently supported)*
    - On Linux: clang and g++
- Support for custom memory management. Memory can be configured such that swapping objects will not break existing references.
- Easily disabled. Define `HSCPP_DISABLE` to turn all hscpp macros into no-ops.
- Optional preprocessor step allows for user-specified file dependencies before compilation.
- Optional dependency tracking parses project `#include` to generate dependency graph.

## System requirements

hscpp uses CMake to build, and requires at least a C++11 compiler. Supported operating systems include Windows, macOS, and Linux. When building below C++17, hscpp will use [ghc filesystem](https://github.com/gulrak/filesystem) to support std::filesystem calls.

## Current status

hscpp is currently in **alpha**, and still undergoing development. Current priorities include:
- Improving testing.
- Improving documentation.
- Using hscpp in a non-toy project.
- Writing general-use tools, such as a high-performance memory manager.
