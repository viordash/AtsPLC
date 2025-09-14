#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/display.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(DisplayTestsGroup){ //
                               TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
}

TEST_TEARDOWN() {
}
}
;

TEST(DisplayTestsGroup, draw_bitmap_from_zero_position) {
    static Bitmap bitmap = {         //
                             { 16,   // width
                               16 }, // height
                             { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                               17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 }
    };

    draw_bitmap(&frame_buffer, 0, 0, &bitmap);

    CHECK_EQUAL(1, frame_buffer.buffer[0]);
    CHECK_EQUAL(2, frame_buffer.buffer[1]);
    CHECK_EQUAL(3, frame_buffer.buffer[2]);
    CHECK_EQUAL(4, frame_buffer.buffer[3]);
    CHECK_EQUAL(8, frame_buffer.buffer[7]);
    CHECK_EQUAL(12, frame_buffer.buffer[11]);
    CHECK_EQUAL(15, frame_buffer.buffer[14]);
    CHECK_EQUAL(16, frame_buffer.buffer[15]);
    CHECK_EQUAL(17, frame_buffer.buffer[0 + DISPLAY_WIDTH]);
    CHECK_EQUAL(18, frame_buffer.buffer[1 + DISPLAY_WIDTH]);
    CHECK_EQUAL(19, frame_buffer.buffer[2 + DISPLAY_WIDTH]);
    CHECK_EQUAL(20, frame_buffer.buffer[3 + DISPLAY_WIDTH]);
    CHECK_EQUAL(24, frame_buffer.buffer[7 + DISPLAY_WIDTH]);
    CHECK_EQUAL(28, frame_buffer.buffer[11 + DISPLAY_WIDTH]);
    CHECK_EQUAL(31, frame_buffer.buffer[14 + DISPLAY_WIDTH]);
    CHECK_EQUAL(32, frame_buffer.buffer[15 + DISPLAY_WIDTH]);
}

TEST(DisplayTestsGroup, draw_bitmap_from_position_0_1) {
    static Bitmap bitmap = {
        //
        { 16,   // width
          16 }, // height
        { 0b11111111, 0b11111110, 0b11111101, 0b11111100, 0b11111011, 0b11111010,
          0b11111001, 0b11111000, 0b11110111, 0b11110110, 0b11110101, 0b11110100,
          0b11110011, 0b11110010, 0b11110001, 0b11110000, //
          0b11101111, 0b11101110, 0b11101101, 0b11101100, 0b11101011, 0b11101010,
          0b11101001, 0b11101000, 0b11100111, 0b11100110, 0b11100101, 0b11100100,
          0b11100011, 0b11100010, 0b11100001, 0b11100000 }
    };

    draw_bitmap(&frame_buffer, 0, 1, &bitmap);

    CHECK_EQUAL(0b11111110, frame_buffer.buffer[0]);
    CHECK_EQUAL(0b11111100, frame_buffer.buffer[1]);
    CHECK_EQUAL(0b11111010, frame_buffer.buffer[2]);
    CHECK_EQUAL(0b11111000, frame_buffer.buffer[3]);
    CHECK_EQUAL(0b11110000, frame_buffer.buffer[7]);
    CHECK_EQUAL(0b11101000, frame_buffer.buffer[11]);
    CHECK_EQUAL(0b11100010, frame_buffer.buffer[14]);
    CHECK_EQUAL(0b11100000, frame_buffer.buffer[15]);
    CHECK_EQUAL(0b11011111, frame_buffer.buffer[0 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11011101, frame_buffer.buffer[1 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11011011, frame_buffer.buffer[2 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11011001, frame_buffer.buffer[3 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11010001, frame_buffer.buffer[7 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11001001, frame_buffer.buffer[11 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11000011, frame_buffer.buffer[14 + DISPLAY_WIDTH]);
    CHECK_EQUAL(0b11000001, frame_buffer.buffer[15 + DISPLAY_WIDTH]);

    CHECK_EQUAL(0b00000001, frame_buffer.buffer[0 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[1 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[2 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[3 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[7 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[11 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[14 + DISPLAY_WIDTH * 2]);
    CHECK_EQUAL(0b00000001, frame_buffer.buffer[15 + DISPLAY_WIDTH * 2]);
}

TEST(DisplayTestsGroup, draw_active_network__with_0_width) {
    CHECK_TRUE(draw_active_network(&frame_buffer, 0, 0, 0));
}

TEST(DisplayTestsGroup, draw_vert_progress_bar__with_0_progress) {
    CHECK_TRUE(draw_vert_progress_bar(&frame_buffer, 0, 0, 0));
}

TEST(DisplayTestsGroup, draw_horz_progress_bar__with_0_progress) {
    CHECK_TRUE(draw_horz_progress_bar(&frame_buffer, 0, 0, 0));
}
