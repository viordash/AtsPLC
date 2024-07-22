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
    CHECK_EQUAL(17, display.buffer[0 + (display.dev.width / 8)]);
    CHECK_EQUAL(18, display.buffer[1 + (display.dev.width / 8)]);
    CHECK_EQUAL(19, display.buffer[2 + (display.dev.width / 8)]);
    CHECK_EQUAL(20, display.buffer[3 + (display.dev.width / 8)]);
    CHECK_EQUAL(24, display.buffer[7 + (display.dev.width / 8)]);
    CHECK_EQUAL(28, display.buffer[11 + (display.dev.width / 8)]);
    CHECK_EQUAL(31, display.buffer[14 + (display.dev.width / 8)]);
    CHECK_EQUAL(32, display.buffer[15 + (display.dev.width / 8)]);
}
