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

TEST_GROUP(MapIOIndicatorTestsGroup){ //
                                 TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;


TEST(MapIOIndicatorTestsGroup, New_progress_value_changed_frame_buffer) {
    MapIOIndicator testable(MapIO::V1);
    Point start_point = {};
    uint8_t progress = 0;
    CHECK_TRUE(testable.Render(&frame_buffer, &start_point, progress));
    frame_buffer.has_changes = false;
    progress = 1;
    CHECK_TRUE(testable.Render(&frame_buffer, &start_point, progress));
    CHECK_TRUE(frame_buffer.has_changes);
}
