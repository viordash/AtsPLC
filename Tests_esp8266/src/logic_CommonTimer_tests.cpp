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
        TestableCommonTimer(uint64_t delay_time_us, InputBase *incoming_item)
            : CommonTimer(incoming_item) {

            this->delay_time_us = delay_time_us;
        }
        virtual ~TestableCommonTimer() {
        }

        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
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
        uint64_t PublicMorozov_start_time_us() {
            return start_time_us;
        }
    };
} // namespace

TEST(LogicCommonTimerTestsGroup, Render_on_top_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, Render_on_bottom_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 1, LogicItemState::lisActive);
    TestableCommonTimer testable(12345, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_when_no_overflowed) {
    volatile uint64_t os_us = 2;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
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
        .expectNCalls(9, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
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

    os_us = INT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 8;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_for_max_delay) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable_0(INT64_MAX, &incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX, left_time);

    os_us = 100;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX - 100, left_time);

    os_us = INT64_MAX - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = INT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = (uint64_t)INT64_MAX + 100;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_for_max_delay_with_time_overflow) {
    volatile uint64_t os_us = UINT64_MAX - 7;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable_0(INT64_MAX, &incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX, left_time);

    os_us = UINT64_MAX - 5;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX - 2, left_time);

    os_us = UINT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX - 7, left_time);

    os_us = 0;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(INT64_MAX - 8, left_time);

    os_us = (uint64_t)INT64_MAX - 9;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = (uint64_t)INT64_MAX - 8;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = (uint64_t)INT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_return_full_delay_if_incoming_item_is_passive) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer prev_element(0, &incomeRail0);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    TestableCommonTimer testable_0(10, &prev_element);

    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;
    os_us = 4;
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(10, left_time);

    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(6, left_time);
}

TEST(LogicCommonTimerTestsGroup, GetProgress) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(10, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable_0(1000, &incomeRail0);
    uint8_t percent04 = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(0, percent04);

    os_us = 1;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(0.4 / 0.4, percent04, 0.5);

    os_us = 5;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(0.8 / 0.4, percent04, 0.5);

    os_us = 100;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(10 / 0.4, percent04, 0.5);

    os_us = 250;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(25 / 0.4, percent04, 0.5);

    os_us = 500;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(50 / 0.4, percent04, 0.5);

    os_us = 990;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(99 / 0.4, percent04, 0.5);

    os_us = 1000;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(100 / 0.4, percent04, 0.5);

    os_us = 1200;
    percent04 = testable_0.PublicMorozov_GetProgress();
    DOUBLES_EQUAL(100 / 0.4, percent04, 0.5);
}

TEST(LogicCommonTimerTestsGroup, DoAction_skip_when_incoming_passive) {
    mock().expectNoCall("esp_timer_get_time");

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisPassive);
    TestableCommonTimer testable(10, &incomeRail0);

    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}

TEST(LogicCommonTimerTestsGroup, DoAction_change_state_to_active_when_timer_raised) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable(10, &incomeRail0);

    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());

    os_us = 10;

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
}

TEST(LogicCommonTimerTestsGroup, does_not_autoreset_after_very_long_period) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable(10, &incomeRail0);

    os_us = 10;

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());

    os_us = 5; //total counter overflow

    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
}

TEST(LogicCommonTimerTestsGroup, DoAction__changing_previous_element_to_active_resets_start_time) {
    volatile uint64_t os_us = 42;
    mock()
        .expectNCalls(4, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail0(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable(10, &incomeRail0);

    CHECK_FALSE(testable.DoAction(false));

    os_us += 19;
    CHECK_EQUAL(42, testable.PublicMorozov_start_time_us());

    CHECK_FALSE(testable.DoAction(true));

    CHECK_EQUAL(42 + 19, testable.PublicMorozov_start_time_us());
}

TEST(LogicCommonTimerTestsGroup, active_previous_element_set_start_time_in_ctor) {
    volatile uint64_t os_us = 42;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    Controller controller(NULL);
    IncomeRail incomeRail_passive(&controller, 0, LogicItemState::lisActive);
    TestableCommonTimer testable(10, &incomeRail_passive);
}

TEST(LogicCommonTimerTestsGroup, passive_previous_element_dont_set_start_time_in_ctor) {
    mock().expectNoCall("esp_timer_get_time");

    Controller controller(NULL);
    IncomeRail incomeRail_passive(&controller, 0, LogicItemState::lisPassive);
    TestableCommonTimer testable(10, &incomeRail_passive);
}