# Computer Architecture Course Final Project - Array Calculation Acceleration

**Acceleration method used**:
- Memory locality: Input matrix reorganized
- Parallel (OpenMP with no more than 4 processing cores): Both gaussian filter and power law transformation.
- Look Up Table: Only in power law transformation.

**Original source file** is `before_optimize.cpp`

**Accelerated source file** is `after_optimize.cpp`

If you want more fun, checkout the playground branch!

## Compile and Run

All things you need to run these file is a up-to-date C++ compiler. If you are using `g++` compiler and `make` tool, that you can use make file here to run the code.

Compile command of `g++` for reference:
```bash
g++ -g -Wall -O0 -std=c++23 -march=native -fopenmp -o after_optimize after_optimize.cpp
```
- `-std=c++23`: Older version of C++ might be compatible but you should keep it as new as possible because lambda expression is only supported in modern C++.

- `-march=native`: If you want a portable executable, remove this option.