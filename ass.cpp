#include <iostream>
#include <experimental/simd>

namespace stdx = std::experimental;

void println(std::string_view name, auto const& a) {
    std::cout << name << ": ";
    for (std::size_t i = 0; i != std::size(a); i++) {
        std::cout << a[i] << ' ';
    }
    std::cout << std::endl;
}

int main(int argc, const char *argv[]) {
    stdx::fixed_size_simd<short int, 16> x([](int i) { return i; });
    stdx::fixed_size_simd<short int, 16> y([](int i) { return 2*i; });
    println("result", x + y);
    return 0;
}