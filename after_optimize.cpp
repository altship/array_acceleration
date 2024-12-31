#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <vectorclass/vectorclass.h>
#include <omp.h>

// Using define rather than variable will be more efficient. ~10ms faster.
#define GAMMA 0.5f

using std::vector;

class FigureProcessor {
private:
    unsigned char **figure;
    unsigned char **result;
    const size_t size;

public:
    FigureProcessor(size_t size, size_t seed = 0) : size(size) {
        
        // !!! Please do not modify the following code !!!
        // 如果你需要修改内存的数据结构，请不要修改初始化的顺序和逻辑
        // 助教可能会通过指定某个初始化seed 的方式来验证你的代码
        // 如果你修改了初始化的顺序，可能会导致你的代码无法通过测试
        std::random_device rd;
        std::mt19937_64 gen(seed == 0 ? rd() : seed);
        std::uniform_int_distribution<unsigned char> distribution(0, 255);
        // !!! ----------------------------------------- !!!

        // 两个数组的初始化在这里，可以改动，但请注意 gen 的顺序是从上到下从左到右即可。
        // Reorganize the memory layout to using continueous memory to improve cache performance.
        // Also add padding to the matrix to avoid boundary check, slightly using extra time here but not obvious
        figure = new unsigned char*[size + 2];
        for (size_t i = 0; i < size + 2; ++i) {
            figure[i] = new unsigned char[size + 2];
        }
        // Filling the matrix area.
        for (size_t i = 1; i < size + 1; ++i) {
            figure[i][0] = figure[i][1] = static_cast<unsigned char>(distribution(gen));
            for (size_t j = 2; j < size + 1; ++j) {
                figure[i][j] = static_cast<unsigned char>(distribution(gen));
            }
            figure[i][size + 1] = figure[i][size];
        }
        // Filling the padding area by using memcpy, this is much faster.
        memcpy(figure[0], figure[1], sizeof(unsigned char) * (size + 2));
        memcpy(figure[size + 1], figure[size], sizeof(unsigned char) * (size + 2));

        result = new unsigned char*[size];
        for (size_t i = 0; i < size; ++i) {
            result[i] = new unsigned char[size];
            memset(result[i], 0, sizeof(unsigned char) * size);
        }
    }


    ~FigureProcessor() {
        // explicit release the matrix memory
        for (size_t i = 0; i < size + 2; ++i) {
            delete[] figure[i];
        }
        delete[] figure;

        for (size_t i = 0; i < size; ++i) {
            delete[] result[i];
        }
        delete[] result;
    }


    // Gaussian filter
    // [[1, 2, 1], [2, 4, 2], [1, 2, 1]] / 16
    void gaussianFilter() {
        // Calculating all the matrix area.
        // Here only rewrite the code for concise(by using padding created above), the calculation is the same as old.
        omp_set_num_threads(4);
#pragma omp parallel for
        for (size_t i = 1; i < size + 1; ++i) {
            for (size_t j = 1; j < size + 1; ++j) {
                result[i - 1][j - 1] =
                        (figure[i - 1][j - 1] + 2 * figure[i - 1][j] +
                         figure[i - 1][j + 1] + 2 * figure[i][j - 1] + 4 * figure[i][j] +
                         2 * figure[i][j + 1] + figure[i + 1][j - 1] +
                         2 * figure[i + 1][j] + figure[i + 1][j + 1]) /
                        16;
            }
        }

        // Four corners are calculated again separately due to different way of averaging.
        // There is no need to avoid calculating these point in previous code, introducing branch is no a wise decision.
        // 处理四个角点
        // 左上角
        result[0][0] = (4 * figure[1][1] + 2 * figure[1][2] + 2 * figure[2][1] +
                                        figure[2][2]) /
                                     9; 

        // 右上角
        result[0][size - 1] = (4 * figure[1][size] + 2 * figure[1][size - 1] +
                                                     2 * figure[2][size] + figure[2][size - 1]) /
                                                    9;

        // 左下角
        result[size - 1][0] = (4 * figure[size][1] + 2 * figure[size][2] +
                                                     2 * figure[size - 1][1] + figure[size - 1][2]) /
                                                    9;

        // 右下角
        result[size - 1][size - 1] =
                (4 * figure[size][size] + 2 * figure[size][size - 1] +
                 2 * figure[size - 1][size] + figure[size - 1][size - 1]) /
                9;
    }


    // Power law transformation
    void powerLawTransformation() {
        // Using LUT to optimize the power law transformation.
        unsigned char lut[256] = {0};

        for (size_t i = 1; i < 256; ++i) {
            float normalized = i / 255.0f;
            lut[i] = static_cast<unsigned char>(255.0f * std::pow(normalized, GAMMA) + 0.5f);
        }

        omp_set_num_threads(4);
#pragma omp parallel for
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                result[i][j] = lut[figure[i + 1][j + 1]];
            }
        }
    }


    // Run benchmark
    unsigned int calcChecksum() {
        unsigned int sum = 0;
        constexpr size_t mod = 1000000007;
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                sum += result[i][j];
                sum %= mod;
            }
        }
        return sum;
    }


    void runBenchmark() {
        auto start = std::chrono::high_resolution_clock::now();
        gaussianFilter();
        auto middle = std::chrono::high_resolution_clock::now();

        unsigned int sum = calcChecksum();

        auto middle2 = std::chrono::high_resolution_clock::now();
        powerLawTransformation();
        auto end = std::chrono::high_resolution_clock::now();

        sum += calcChecksum();
        sum %= 1000000007;
        std::cout << "Checksum: " << sum << "\n";

        auto milliseconds =
                std::chrono::duration_cast<std::chrono::milliseconds>(middle - start) +
                std::chrono::duration_cast<std::chrono::milliseconds>(end - middle2);
        std::cout << "Benchmark time: " << milliseconds.count() << " ms\n";
    }
};


// Main function
// !!! Please do not modify the main function !!!
int main(int argc, const char **argv) {
    constexpr size_t size = 16384;
    FigureProcessor processor(size, argc > 1 ? std::stoul(argv[1]) : 0);
    processor.runBenchmark();
    return 0;
}
