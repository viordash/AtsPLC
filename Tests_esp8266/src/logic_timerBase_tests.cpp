#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/TimerBase.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

static const Bitmap bitmap_active = { //
    { 24,                             // width
      16 },                           // height
    { 0xFF, 0x00, 0x01, 0x01, 0x3F, 0x01, 0x01, 0x00, 0x12, 0x25, 0x29, 0x12,
      0x00, 0x1E, 0x29, 0x29, 0x2E, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x00, 0xFF,
      0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF }
};

static const Bitmap bitmap_passive = { //
    { 24,                              // width
      16 },                            // height
    { 0xDB, 0x00, 0x01, 0x01, 0x3F, 0x01, 0x01, 0x00, 0x12, 0x25, 0x29, 0x12,
      0x00, 0x1E, 0x29, 0x29, 0x2E, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x00, 0xDB,
      0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6 }
};

TEST_GROUP(LogicTimerBaseTestsGroup){ //
                                      TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

class TestableTimerBase : public TimerBase {
  public:
    TestableTimerBase(uint32_t delay_time_s, InputBase &prev_item) : TimerBase(prev_item) {
        str_size = sprintf(this->str_time, "%u", delay_time_s);
    }
    virtual ~TestableTimerBase() {
    }

    const Bitmap *GetCurrentBitmap() override final {
        switch (state) {
            case LogicItemState::lisActive:
                return &bitmap_active;

            default:
                return &bitmap_passive;
        }
    }

    bool DoAction() override final {
        return true;
    }
};

TEST(LogicTimerBaseTestsGroup, Render_on_top_network) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableTimerBase testable(12345, incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicTimerBaseTestsGroup, Render_on_bottom_network) {

    Controller controller;
    IncomeRail incomeRail(controller, 1);
    TestableTimerBase testable(12345, incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}
