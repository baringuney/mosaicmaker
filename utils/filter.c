#include "../yesilbaglastirici.h"

unsigned char getColor(unsigned char rgb, int* range) {
    // Clamp the range values to [0, 255]
    if (range[0] < 0) range[0] = 0;
    if (range[1] > 255) range[1] = 255;

    // Calculate the difference and ratio
    int range_diff = range[1] - range[0];  // Use int for calculation

    // Calculate the result without floating-point arithmetic
    unsigned char result = (unsigned char)((range_diff * rgb) / 256 + range[0]);
    return result;
}

