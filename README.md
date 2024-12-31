# Computer Architecture Course Final Project - Array Calculation Acceleration

**Acceleration method used**:
- Memory locality: Input matrix reorganized
- Parallel (OpenMP with no more than 4 processing cores): Both gaussian filter and power law transformation.
- Look Up Table: Only in power law transformation.

**Original source file** is `before_optimize.cpp`

**Accelerated source file** is `after_optimize.cpp`

Optional Header: 
- Vectorclass: https://github.com/vectorclass/version2
    - This is a modern vector calculation library that uses modern X86 CPU's SIMD technology and protect you from directly get into CPU Intrinsics.

    - Although the final code is not intended to use this library due to high memory latency, I have tried many interesting things with it, so it will be kept and you can have some fun with my last but one commit(you can find it by my commit message) and that reserved all the things I tried.

`im2col.py` you might see in previous commit: A efficient way to do the image to convolution suitable matrix transformation done by Stanford CS231N course member, pasted here is only for reference. It is also a interesting algorithm.
Website:https://cs231n.github.io/assignments2024/assignment2/

## Compile and Run

All things you need to run these file is a up-to-date C++ compiler. If you are using `g++` compiler and `make` tool, that you can use make file here to run the code.

Compile command of `g++` for reference:
```bash
g++ -g -Wall -O0 -std=c++23 -mavx2 -march=native -fopenmp -I. -o after_optimize after_optimize.cpp
```
- `-std=c++23`: Older version of C++ might be compatible but you should keep it as new as possible because lambda expression is only supported in modern C++.

- `-march=native`: If you want a portable executable, remove this option.

- `-mavx2` is a local machine specified option that means using AVX2 technology for SIMD, you can replace or remove it to suit your machine. NOTE: Using technology that your machine do not support may cause compile error.

- If you **not intended** to use vectorclass header, you can remove `-I.` and `-mavx2` from the compile command.