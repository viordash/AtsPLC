#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "esp_timer.h"
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
            this->raise_time_us = (uint64_t)esp_timer_get_time() + delay_time_us;
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
        uint64_t PublicMorozov_GetLeftTime() {
            return GetLeftTime();
        }
        uint8_t PublicMorozov_GetProgress() {
            return GetProgress();
        }
    };
} // namespace

TEST(LogicCommonTimerTestsGroup, Render_on_top_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, Render_on_bottom_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    Controller controller;
    IncomeRail incomeRail(controller, 1);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, IncomingItemStateHasChanged_has_single_responsibility) {
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();

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

TEST(LogicCommonTimerTestsGroup, GetLeftTime_when_no_overflowed) {
    volatile uint64_t os_us = 2;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableCommonTimer testable_0(10, &incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(10, left_time);

    os_us = 10;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(2, left_time);

    os_us = 12 - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = 12;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = 12 + 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 1LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_when_is_overflowed) {
    volatile uint64_t os_us = UINT64_MAX - 7;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableCommonTimer testable_0(10, &incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(10, left_time);

    os_us = UINT64_MAX - 4;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(7, left_time);

    os_us = UINT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(3, left_time);

    os_us = 0;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(3 - 1, left_time);

    os_us = 3 - 2;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = 3 - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 8;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetProgress) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(10, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableCommonTimer testable_0(100, &incomeRail0);
    uint8_t percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(0, percent);

    os_us = 1;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(1, percent);

    os_us = 5;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(5, percent);

    os_us = 10;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(10, percent);

    os_us = 25;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(25, percent);

    os_us = 50;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(50, percent);

    os_us = 99;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(99, percent);

    os_us = 100;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(100, percent);

    os_us = 120;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(100, percent);
}