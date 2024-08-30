#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/MapIOIndicator.h"
#include "main/LogicProgram/StatusBar.cpp"
#include "main/LogicProgram/StatusBar.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(StatusBarTestsGroup){ //
                                 TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

class TestableStatusBar : public StatusBar {
  public:
    explicit TestableStatusBar(uint8_t y) : StatusBar(y) {
    }
    virtual ~TestableStatusBar() {
    }
};

TEST(StatusBarTestsGroup, Total_width_not_excess_display_size) {
    TestableStatusBar testable(0);
    CHECK_TRUE(testable.Render(frame_buffer));

    const int component_area = DISPLAY_WIDTH * 2;
    for (int i = component_area; i < DISPLAY_WIDTH * DISPLAY_HEIGHT / 8; i++) {
        CHECK_EQUAL(0, frame_buffer[i]);
    }
}
