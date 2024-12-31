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

// // These conversion only used in SIMD method.
// inline void convert_uchar_to_ushort(const unsigned char* input, unsigned short* output, size_t size);
// inline void convert_ushort_to_uchar(const unsigned short* input, unsigned char* output, size_t size);

class FigureProcessor {
private:
    // vector<vector<unsigned char>> figure;
    // vector<vector<unsigned char>> result;
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
        // for (size_t i = 0; i < size; ++i) {
        //     figure.push_back(vector<unsigned char>());
        //     for (size_t j = 0; j < size; ++j) {
        //         figure[i].push_back(static_cast<unsigned char>(distribution(gen)));
        //     }
        // }

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

        // for (size_t i = 0; i < size + 2; ++i) {
        //     for (size_t j = 0; j < size + 2; ++j) {
        //         std::cout << static_cast<int>(figure[i][j]) << " ";
        //     }
        //     std::cout << std::endl;
        // }

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
        // // Here are some code snippets for using SIMD instructions to optimize this function.
        // // This one is using type conversion for unsigned char to unsigned short for avoiding overflow.
        // unsigned short* row1 = new unsigned short[size + 2];
        // unsigned short* row2 = new unsigned short[size + 2];
        // unsigned short* row3 = new unsigned short[size + 2];

        // convert_uchar_to_ushort(figure[0], row1, size + 2);
        // convert_uchar_to_ushort(figure[1], row2, size + 2);
        // convert_uchar_to_ushort(figure[2], row3, size + 2);
        
        // Vec16us vec[9];
        // Vec16us ker[9];
        // Vec16us res(0);
        // ker[0] = ker[2] = ker[6] = ker[8] = Vec16us(1);
        // ker[1] = ker[3] = ker[5] = ker[7] = Vec16us(2);
        // ker[4] = Vec16us(4);

        // for (size_t i = 0; i < size; i += 16) {
        //     for (size_t j = 0; j < 3; j++) {
        //         vec[j].load(row1 + i + j);
        //         vec[j + 3].load(row2 + i + j);
        //         vec[j + 6].load(row3 + i + j);
        //     }
            
        //     res = 0;            
        //     for (size_t j = 0; j < 9; j++) {
        //         res += vec[j] * ker[j];
        //     }
        //     res = res / 16;
        //     res.store(row1 + i);
        // }

        // convert_ushort_to_uchar(row1, result[0], size);

        // for (size_t k = 1; k < size; k++) {
        //     memcpy(row1, row2, sizeof(unsigned short) * (size + 2));
        //     memcpy(row2, row3, sizeof(unsigned short) * (size + 2));
        //     convert_uchar_to_ushort(figure[k + 2], row3, size + 2);

        //     for (size_t i = 0; i < size; i += 16) {
        //         for (size_t j = 0; j < 3; j++) {
        //             vec[j].load(row1 + i + j);
        //             vec[j + 3].load(row2 + i + j);
        //             vec[j + 6].load(row3 + i + j);
        //         }
                
        //         res = 0;            
        //         for (size_t j = 0; j < 9; j++) {
        //             res += vec[j] * ker[j];
        //         }
        //         res = res / 16;
        //         res.store(row1 + i);
        //     }

        //     convert_ushort_to_uchar(row1, result[k], size);
        // }

        // delete[] row1;
        // delete[] row2;
        // delete[] row3;


        // // This one is using Vec32uc to load the data, and using Vec32uc to store the result.
        // // Be cautions that doing 8bit arithmatic add is slow. For more, see the vectorclass manuel.
        // // No type conversion is used here, severe overflow occured here.
        // Vec32uc vec[9];
        // Vec32uc ker[9];
        // Vec32uc res(0);
        // ker[0] = ker[2] = ker[6] = ker[8] = Vec32uc(1);
        // ker[1] = ker[3] = ker[5] = ker[7] = Vec32uc(2);
        // ker[4] = Vec32uc(4);

        // for (size_t i = 0; i < size; i += 32) {
        //     for (size_t j = 0; j < 3; j++) {
        //         vec[j].load(figure[0] + i + j);
        //         vec[j + 3].load(figure[1] + i + j);
        //         vec[j + 6].load(figure[2] + i + j);
        //     }
            
        //     res = 0;            
        //     for (size_t j = 0; j < 9; j++) {
        //         res += vec[j] * ker[j];
        //     }
        //     res = res / 16;
        //     res.store(result[0] + i);
        // }

        // for (size_t k = 1; k < size; k++) {
        //     for (size_t i = 0; i < size; i += 32) {
        //         for (size_t j = 0; j < 3; j++) {
        //             vec[j].load(figure[k] + i + j);
        //             vec[j + 3].load(figure[k + 1] + i + j);
        //             vec[j + 6].load(figure[k + 2] + i + j);
        //         }
                
        //         res = 0;            
        //         for (size_t j = 0; j < 9; j++) {
        //             res += vec[j] * ker[j];
        //         }
        //         res = res / 16;
        //         res.store(result[k] + i);
        //     }
        // }
        
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

        // for (size_t i = 1; i < size - 1; ++i) {
        //     result[i][0] =
        //             (figure[i - 1][0] + 2 * figure[i - 1][0] + figure[i - 1][1] +
        //              2 * figure[i][0] + 4 * figure[i][0] + 2 * figure[i][1] +
        //              figure[i + 1][0] + 2 * figure[i + 1][0] + figure[i + 1][1]) /
        //             16;

        //     result[i][size - 1] =
        //             (figure[i - 1][size - 2] + 2 * figure[i - 1][size - 1] +
        //              figure[i - 1][size - 1] + 2 * figure[i][size - 2] +
        //              4 * figure[i][size - 1] + 2 * figure[i][size - 1] +
        //              figure[i + 1][size - 2] + 2 * figure[i + 1][size - 1] +
        //              figure[i + 1][size - 1]) /
        //             16;
        // }

        // for (size_t j = 1; j < size - 1; ++j) {
        //     result[0][j] =
        //             (figure[0][j - 1] + 2 * figure[0][j] + figure[0][j + 1] +
        //              2 * figure[0][j - 1] + 4 * figure[0][j] + 2 * figure[0][j + 1] +
        //              figure[1][j - 1] + 2 * figure[1][j] + figure[1][j + 1]) /
        //             16;

        //     result[size - 1][j] =
        //             (figure[size - 2][j - 1] + 2 * figure[size - 2][j] +
        //              figure[size - 2][j + 1] + 2 * figure[size - 1][j - 1] +
        //              4 * figure[size - 1][j] + 2 * figure[size - 1][j + 1] +
        //              figure[size - 1][j - 1] + 2 * figure[size - 1][j] +
        //              figure[size - 1][j + 1]) /
        //             16;
        // }

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
        // for (size_t i = 0; i < size; ++i) {
        //     for (size_t j = 0; j < size; ++j) {
        //         std::cout << static_cast<int>(result[i][j]) << " ";
        //     }
        //     std::cout << "\n";
        // }
    }


    // Power law transformation
    void powerLawTransformation() {
        // constexpr float gamma = 0.5f;

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
        
        // for (size_t i = 1; i < size + 1; ++i) {
        //     for (size_t j = 1; j < size + 1; ++j) {
        //         if(figure[i][j] == 0) {
        //             result[i - 1][j - 1] = 0;
        //             continue;
        //         }
        //         float normalized = (figure[i][j]) / 255.0f;
        //         result[i - 1][j - 1] = static_cast<unsigned char>(
        //                 255.0f * std::pow(normalized, gamma) + 0.5f); 
        //     }
        // }
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


// // These conversion only used in SIMD method.
// // Using inline function to avoid function call overhead.
// inline void convert_uchar_to_ushort(const unsigned char* input, unsigned short* output, size_t size) {
//     std::transform(input, input + size, output, [](unsigned char x) { return static_cast<unsigned short>(x); });
// }

// inline void convert_ushort_to_uchar(const unsigned short* input, unsigned char* output, size_t size) {
//     std::transform(input, input + size, output, [](unsigned short x) { return static_cast<unsigned char>(x); });
// }


// Main function
// !!! Please do not modify the main function !!!
int main(int argc, const char **argv) {
    constexpr size_t size = 16384;
    FigureProcessor processor(size, argc > 1 ? std::stoul(argv[1]) : 0);
    processor.runBenchmark();
    return 0;
}
