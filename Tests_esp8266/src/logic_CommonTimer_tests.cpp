#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonTimer.h"
#include "main/LogicProgram/Inputs/IncomeRail.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicCommonTimerTestsGroup){ //
                                        TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    static const Bitmap bitmap = { //
        { 16,                      // width
          16 },                    // height
        { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
          0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
    };

    class TestableCommonTimer : public CommonTimer {
      public:
        TestableCommonTimer(uint32_t delay_time_us, InputBase *incoming_item)
            : CommonTimer(incoming_item) {

            this->delay_time_us = delay_time_us;
        }
        virtual ~TestableCommonTimer() {
        }

        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        bool DoAction() {
            return true;
        }
        bool PublicMorozov_IncomingItemStateHasChanged() {
            return IncomingItemStateHasChanged();
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicCommonTimerTestsGroup, Render_on_top_network) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, Render_on_bottom_network) {

    Controller controller;
    IncomeRail incomeRail(controller, 1);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, IncomingItemStateHasChanged_has_single_responsibility) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableCommonTimer prev_element(0, &incomeRail0);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    TestableCommonTimer testable(10, &prev_element);
    CHECK_FALSE(testable.PublicMorozov_IncomingItemStateHasChanged());

    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.PublicMorozov_IncomingItemStateHasChanged());
    CHECK_FALSE(testable.PublicMorozov_IncomingItemStateHasChanged());

    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    CHECK_TRUE(testable.PublicMorozov_IncomingItemStateHasChanged());
    CHECK_FALSE(testable.PublicMorozov_IncomingItemStateHasChanged());
}