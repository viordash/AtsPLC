#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/StatusBar.cpp"
#include "main/Display/StatusBar.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(StatusBarTestsGroup){ //
                                 TEST_SETUP(){}

                                 TEST_TEARDOWN(){}
};

class TestableStatusBar : public StatusBar {
  public:
    explicit TestableStatusBar(uint8_t y) : StatusBar(y) {
    }
    virtual ~TestableStatusBar() {
    }
};

TEST(StatusBarTestsGroup, Total_width_not_excess_display_size) {
    TestableStatusBar testable(0);
    testable.Render(frame_buffer);
}
