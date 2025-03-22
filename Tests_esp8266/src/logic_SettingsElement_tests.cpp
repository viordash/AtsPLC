#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Settings/SettingsElement.cpp"
#include "main/LogicProgram/Settings/SettingsElement.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicSettingsElementTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
Controller::Start(NULL, NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
    mock().enable();
}
}
;

namespace {
    class TestableSettingsElement : public SettingsElement {
      public:
        TestableSettingsElement() : SettingsElement() {
        }
    };
} // namespace

TEST(LogicSettingsElementTestsGroup, Render) {
    TestableSettingsElement testable;

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(113, start_point.x);
}
