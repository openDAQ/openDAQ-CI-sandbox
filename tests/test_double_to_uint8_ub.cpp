#include <iostream>
#include <cstdint>

int main() {
    double value = 444.0;

    std::cout << "Converting double " << value << " to uint8_t (10 times):\n";

    for (int i = 0; i < 10; ++i) {
        uint8_t result = (uint8_t)(value);
        std::cout << "  Attempt " << i << ": " << (int)result << "\n";
    }

    std::cout << "\nConverting in expressions (10 times):\n";
    for (int i = 0; i < 10; ++i) {
        double d = 40.0 * 11.1;
        uint8_t result = (uint8_t)(d);
        std::cout << "  40.0 * 11.1 = " << d << " -> uint8_t = " << (int)result << "\n";
    }

    std::cout << "\nComparing two conversions:\n";
    for (int i = 0; i < 10; ++i) {
        uint8_t a = (uint8_t)(40.0 * 11.1);
        uint8_t b = (uint8_t)(40.0 * 11.1);
        std::cout << "  a=" << (int)a << ", b=" << (int)b << ", equal=" << (a == b ? "yes" : "NO!") << "\n";
    }

    return 0;
}
