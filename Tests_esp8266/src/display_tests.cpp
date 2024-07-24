#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/display.cpp"

TEST_GROUP(DisplayTestsGroup){ //
                               TEST_SETUP(){ memset(display.buffer, 0, sizeof(display.buffer));
}

TEST_TEARDOWN() {
}
}
;

TEST(DisplayTestsGroup, draw_xbm_from_zero_position) {
    uint8_t xbm_height = 16;
    uint8_t xbm_width = 16;
    uint8_t xbm_data[] = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                           17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

    draw_xbm(&display.dev, display.buffer, 0, 0, xbm_data, xbm_height, xbm_width);

    CHECK_EQUAL(1, display.buffer[0]);
    CHECK_EQUAL(2, display.buffer[1]);
    CHECK_EQUAL(3, display.buffer[2]);
    CHECK_EQUAL(4, display.buffer[3]);
    CHECK_EQUAL(8, display.buffer[7]);
    CHECK_EQUAL(12, display.buffer[11]);
    CHECK_EQUAL(15, display.buffer[14]);
    CHECK_EQUAL(16, display.buffer[15]);
    CHECK_EQUAL(17, display.buffer[0 + display.dev.width]);
    CHECK_EQUAL(18, display.buffer[1 + display.dev.width]);
    CHECK_EQUAL(19, display.buffer[2 + display.dev.width]);
    CHECK_EQUAL(20, display.buffer[3 + display.dev.width]);
    CHECK_EQUAL(24, display.buffer[7 + display.dev.width]);
    CHECK_EQUAL(28, display.buffer[11 + display.dev.width]);
    CHECK_EQUAL(31, display.buffer[14 + display.dev.width]);
    CHECK_EQUAL(32, display.buffer[15 + display.dev.width]);
}

TEST(DisplayTestsGroup, draw_xbm_from_position_0_1) {
    uint8_t xbm_height = 16;
    uint8_t xbm_width = 16;
    uint8_t xbm_data[] = { 0b11111111, 0b11111110, 0b11111101, 0b11111100, 0b11111011, 0b11111010,
                           0b11111001, 0b11111000, 0b11110111, 0b11110110, 0b11110101, 0b11110100,
                           0b11110011, 0b11110010, 0b11110001, 0b11110000, //
                           0b11101111, 0b11101110, 0b11101101, 0b11101100, 0b11101011, 0b11101010,
                           0b11101001, 0b11101000, 0b11100111, 0b11100110, 0b11100101, 0b11100100,
                           0b11100011, 0b11100010, 0b11100001, 0b11100000 };

    draw_xbm(&display.dev, display.buffer, 0, 1, xbm_data, xbm_height, xbm_width);

    CHECK_EQUAL(0b11111110, display.buffer[0]);
    CHECK_EQUAL(0b11111100, display.buffer[1]);
    CHECK_EQUAL(0b11111010, display.buffer[2]);
    CHECK_EQUAL(0b11111000, display.buffer[3]);
    CHECK_EQUAL(0b11110000, display.buffer[7]);
    CHECK_EQUAL(0b11101000, display.buffer[11]);
    CHECK_EQUAL(0b11100010, display.buffer[14]);
    CHECK_EQUAL(0b11100000, display.buffer[15]);
    CHECK_EQUAL(0b11011111, display.buffer[0 + display.dev.width]);
    CHECK_EQUAL(0b11011101, display.buffer[1 + display.dev.width]);
    CHECK_EQUAL(0b11011011, display.buffer[2 + display.dev.width]);
    CHECK_EQUAL(0b11011001, display.buffer[3 + display.dev.width]);
    CHECK_EQUAL(0b11010001, display.buffer[7 + display.dev.width]);
    CHECK_EQUAL(0b11001001, display.buffer[11 + display.dev.width]);
    CHECK_EQUAL(0b11000011, display.buffer[14 + display.dev.width]);
    CHECK_EQUAL(0b11000001, display.buffer[15 + display.dev.width]);

    CHECK_EQUAL(0b00000001, display.buffer[0 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[1 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[2 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[3 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[7 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[11 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[14 + display.dev.width * 2]);
    CHECK_EQUAL(0b00000001, display.buffer[15 + display.dev.width * 2]);
}
