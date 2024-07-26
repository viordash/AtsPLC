#pragma once

#include <stdint.h>
#include <unistd.h>

struct Point {
    uint8_t x;
    uint8_t y;
};

struct Size {
    uint8_t width;
    uint8_t height;
};

struct Bitmap {
    const Size size;
    const uint8_t data[];
};