
# ToyCComp

ToyCComp is a hobby project for a C compiler aimed at learning both the C programming language and the fundamentals of compiler design. The ultimate goal is to create a self-compiling compiler (a compiler capable of compiling its own source code) that utilizes SSA (Static Single Assignment) for advanced optimizations.

# Test Status

![Test Status](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/mgtm98/ToyCComp/main/badge.json&label=Tests&query=message&color=green)


## Goals
- **Learn Compiler Design**: Understand the theory and implementation of modern compilers.
- **Learn C**: Gain deeper insights into C through practical application.
- **Implement SSA**: Leverage SSA for code analysis and optimization.
- **Optimization Techniques**: Include optimizations like:
  - Dead Code Elimination
  - Loop Unrolling
  - Function Inlining (and potentially more as the project evolves)

## Features
- **BNF Grammar**: A simple grammar implementation. (See [ToyCComp_BNF.txt](ToyCComp_BNF.txt) for details.)
- **Code Optimizations**: Begin with basic optimization techniques, expanding over time.
- **Intermediate Representation (IR)**: Introduce an IR as the project progresses to facilitate SSA and optimizations.
- **Self-Compilation**: The compiler will eventually compile its own source code.
- **Intel x86 Assembly**: The compiler generates assembly in Intel syntax, compatible with the NASM assembler.


## Getting Started
This project is in its early stages. Contributions, suggestions, and discussions are welcome!

### Requirements
- C Compiler (e.g., GCC, Clang)
- NASM Assembler
- Makefiles for building the project

### Build Instructions
Use the provided Makefile to build the project:
```bash
make run
```

## Inspiration
This project is heavily inspired by [DoctorWkt's `acwj`](https://github.com/DoctorWkt/acwj). However, ToyCComp introduces several modifications and extensions to the original design, including support for advanced optimizations and SSA-based compilation.

## Future Plans
- Extend the grammar to support more complex C constructs.
- Develop a custom intermediate representation for code analysis and transformations.
- Implement SSA-based optimizations and further techniques as the compiler matures.
- Create robust error handling for the compiler.

## Contributions
This is a personal learning project, but contributions are welcome. Feel free to open issues or submit pull requests to enhance the project.

## License
MIT License. See `LICENSE` for more information.
