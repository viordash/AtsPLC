#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/MapIOIndicator.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(MapIOIndicatorTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

TEST(MapIOIndicatorTestsGroup, Render) {
    MapIOIndicator testable(MapIO::V1);
    Point start_point = {};
    uint8_t progress = 10;
    testable.Render(&frame_buffer, &start_point, progress);

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++) {
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}