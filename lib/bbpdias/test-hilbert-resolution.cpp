#include <iostream>

#include "libbbpdias/Hilbert.hpp"

enum { hilbert_bits = 12 };

static unsigned long pow (int base, int exp)
{
    unsigned long result = 1;
    for (int i = 1; i <= exp; i++)
    {
        result *= base;
    }
    return result;
}

int main(void)
{
    bitmask_t coords[3];
    coords[0] = 2;
    coords[1] = 2;
    coords[2] = 2;

    bitmask_t initial_hilbert_value = hilbert_c2i (3, hilbert_bits, coords);
    std::cout << "coords = " << coords[0] << ' ' << coords[1] << ' '
              << coords[2] << '\n';
    std::cout << "hilbert value = " << initial_hilbert_value << '\n';

    bitmask_t hilbert_value = initial_hilbert_value;
    while (hilbert_value == initial_hilbert_value)
    {
        coords[1]++;
        hilbert_value = hilbert_c2i (3, hilbert_bits, coords);
    }

    std::cout << "First change:\n";
    std::cout << "coords = " << coords[0] << ' ' << coords[1] << ' '
              << coords[2] << '\n';
    std::cout << "hilbert value = " << hilbert_value << '\n';

    initial_hilbert_value = hilbert_value;
    bitmask_t range_start = coords[1];

    while (hilbert_value == initial_hilbert_value)
    {
        coords[1]++;
        hilbert_value = hilbert_c2i (3, hilbert_bits, coords);
    }

    std::cout << "Second change:\n";
    std::cout << "coords = " << coords[0] << ' ' << coords[1] << ' '
              << coords[2] << '\n';
    std::cout << "hilbert value = " << hilbert_value << '\n';
    std::cout << "Value range = " << coords[1] - range_start << '\n';

    std::cout << "First ten Hilbert curve points with " << hilbert_bits
              << " bits:\n";
    for (bitmask_t i = 0; i < 10; i++)
    {
        bitmask_t p[3];
        hilbert_i2c (3, hilbert_bits, i, p);
        std::cout << p[0] << ' ' << p[1] << ' ' << p[2] << '\n';
    }
    std::cout << "Last point:\n";
    bitmask_t p[3];
    bitmask_t i = pow(8, hilbert_bits);
    std::cout << "Index: " << i << '\n';
    hilbert_i2c (3, hilbert_bits, i, p);
    std::cout << p[0] << ' ' << p[1] << ' ' << p[2] << '\n';
}
