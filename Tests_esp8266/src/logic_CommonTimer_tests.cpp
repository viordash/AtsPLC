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
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Inputs/TimerMSecs.h"
#include "main/LogicProgram/Inputs/TimerSecs.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

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
        TestableCommonTimer(uint64_t delay_time_us) : CommonTimer() {

            this->delay_time_us = delay_time_us;
            str_size = sprintf(this->str_time, "%u", (uint32_t)delay_time_us);
        }
        virtual ~TestableCommonTimer() {
        }

        const Bitmap *GetCurrentBitmap(LogicItemState state) {
            (void)state;
            return &bitmap;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        uint64_t PublicMorozov_GetLeftTime() {
            return GetLeftTime();
        }
        uint8_t PublicMorozov_GetProgress(LogicItemState prev_elem_state) {
            return GetProgress(prev_elem_state);
        }
        uint64_t PublicMorozov_start_time_us() {
            return start_time_us;
        }

        size_t Serialize(uint8_t *buffer, size_t buffer_size) override {
            (void)buffer;
            (void)buffer_size;
            return 0;
        }
        size_t Deserialize(uint8_t *buffer, size_t buffer_size) override {
            (void)buffer;
            (void)buffer_size;
            return 0;
        }
        TvElementType GetElementType() override {
            return TvElementType::et_Undef;
        }
        void SelectNext() override {
        }
        void SelectPrior() override {
        }
        void PageUp() override {
        }
        void PageDown() override {
        }
        void Change() {
        }
        bool EditingCompleted() {
            return true;
        }
    };
} // namespace

TEST(LogicCommonTimerTestsGroup, Render_on_top_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    TestableCommonTimer testable(12345);

    Point start_point = { 0, 0 };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));
}

TEST(LogicCommonTimerTestsGroup, Render_on_bottom_network) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    TestableCommonTimer testable(12345);

    Point start_point = { 0, 0 };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));
}

TEST(LogicCommonTimerTestsGroup, GetLeftTime_when_no_overflowed) {
    volatile uint64_t os_us = 2;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable_0(10);
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

    TestableCommonTimer testable_0(10);
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

    TestableCommonTimer testable_0(INT64_MAX);
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

    TestableCommonTimer testable_0(INT64_MAX);
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

TEST(LogicCommonTimerTestsGroup, GetProgress) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(10, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable_0(1000);
    uint8_t percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    CHECK_EQUAL(0, percent04);

    os_us = 1;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(0.4 / 0.4, percent04, 0.5);

    os_us = 5;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(0.8 / 0.4, percent04, 0.5);

    os_us = 100;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(10 / 0.4, percent04, 0.5);

    os_us = 250;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(25 / 0.4, percent04, 0.5);

    os_us = 500;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(50 / 0.4, percent04, 0.5);

    os_us = 990;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(99 / 0.4, percent04, 0.5);

    os_us = 1000;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(100 / 0.4, percent04, 0.5);

    os_us = 1200;
    percent04 = testable_0.PublicMorozov_GetProgress(LogicItemState::lisActive);
    DOUBLES_EQUAL(100 / 0.4, percent04, 0.5);
}

TEST(LogicCommonTimerTestsGroup, DoAction_skip_when_incoming_passive) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    TestableCommonTimer testable(10);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();

    TestableCommonTimer testable(10);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
    CHECK_FALSE_TEXT(testable.DoAction(false, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
}

TEST(LogicCommonTimerTestsGroup, DoAction_change_state_to_active_when_timer_raised) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable(10);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    os_us = 10;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, does_not_autoreset_after_very_long_period) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable(10);

    os_us = 10;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    os_us = 5; //total counter overflow

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, DoAction__changing_previous_element_to_active_resets_start_time) {
    volatile uint64_t os_us = 42;
    mock()
        .expectNCalls(4, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable(10);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));

    os_us += 19;
    CHECK_EQUAL(42, testable.PublicMorozov_start_time_us());

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive));

    CHECK_EQUAL(42 + 19, testable.PublicMorozov_start_time_us());
}

TEST(LogicCommonTimerTestsGroup, set_start_time_in_ctor) {
    volatile uint64_t os_us = 42;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableCommonTimer testable(10);
}

TEST(LogicCommonTimerTestsGroup, TryToCast) {
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    InputNC inputNC;
    CHECK_TRUE(CommonTimer::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(CommonTimer::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(CommonTimer::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(CommonTimer::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(CommonTimer::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(CommonTimer::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(CommonTimer::TryToCast(&comparatorLs) == NULL);

    TimerMSecs timerMSecs;
    CHECK_TRUE(CommonTimer::TryToCast(&timerMSecs) == &timerMSecs);

    TimerSecs timerSecs;
    CHECK_TRUE(CommonTimer::TryToCast(&timerSecs) == &timerSecs);

    DirectOutput directOutput;
    CHECK_TRUE(CommonTimer::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(CommonTimer::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(CommonTimer::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(CommonTimer::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(CommonTimer::TryToCast(&decOutput) == NULL);
}