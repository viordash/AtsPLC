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

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
Controller::Start(NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
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
    TestableCommonTimer testable(12345);

    Point start_point = { 0, 0 };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));
}

TEST(LogicCommonTimerTestsGroup, Render_on_bottom_network) {
    TestableCommonTimer testable(12345);

    Point start_point = { 0, 0 };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));
}

TEST(LogicCommonTimerTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableCommonTimer testable(10);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
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
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(4, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableCommonTimer testable(10 * portTICK_PERIOD_MS * 1000);

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    ticks += 9;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    ticks += 1;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, does_not_autoreset_after_very_long_period) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(4, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableCommonTimer testable(20 * portTICK_PERIOD_MS * 1000);
    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive));

    ticks += 20;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    ticks = 9000; //total counter overflow;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicCommonTimerTestsGroup, DoAction__changing_previous_element_to_active_resets_start_time) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(5, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableCommonTimer testable(20 * portTICK_PERIOD_MS * 1000);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));

    ticks += 100;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive));

    ticks += 19;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));

    ticks += 1;
    Controller::RemoveExpiredWakeupRequests();

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
}

TEST(LogicCommonTimerTestsGroup, TryToCast) {
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