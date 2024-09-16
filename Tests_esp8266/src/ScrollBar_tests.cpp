#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/ScrollBar.cpp"
#include "main/Display/ScrollBar.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(ScrollBarTestsGroup){ //
                                 TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

TEST(ScrollBarTestsGroup, Render) {

    CHECK_TRUE(ScrollBar::Render(frame_buffer, 4, 2, 0));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}

TEST(ScrollBarTestsGroup, Skip_render_if_nothing_to_scroll) {
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 40, 40, 0));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_FALSE(any_pixel_coloring);
}

TEST(ScrollBarTestsGroup, No_screen_overflow) {
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 0, 0, 0));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 100, 2, 0));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 100, 2, 1));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 100, 2, 99));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 4, 2, 0));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 4, 2, 1));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 4, 2, 2));
    CHECK_TRUE(ScrollBar::Render(frame_buffer, 3, 2, 0));

    bool any_overflow_pixel = false;
    size_t possible_line1_position_in_buffer =
        SCROLLBAR_LEFT + (DISPLAY_WIDTH * (INCOME_RAIL_TOP / 8));
    size_t possible_line2_position_in_buffer = possible_line1_position_in_buffer + 1;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            if (i == possible_line1_position_in_buffer) {
                possible_line1_position_in_buffer += DISPLAY_WIDTH;
                continue;
            }
            if (i == possible_line2_position_in_buffer) {
                possible_line2_position_in_buffer += DISPLAY_WIDTH;
                continue;
            }
            any_overflow_pixel = true;
            break;
        }
    }
    CHECK_FALSE(any_overflow_pixel);
}