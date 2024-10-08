#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/TimerSecs.cpp"
#include "main/LogicProgram/Inputs/TimerSecs.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicTimerSecsTestsGroup){ //
                                      TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableTimerSecs : public TimerSecs {
      public:
        TestableTimerSecs() : TimerSecs() {
        }
        virtual ~TestableTimerSecs() {
        }

        uint64_t PublicMorozov_GetDelayTimeUs() {
            return delay_time_us;
        }
        uint8_t PublicMorozov_GetProgress(LogicItemState prev_elem_state) {
            return GetProgress(prev_elem_state);
        }
        uint8_t PublicMorozov_ProgressHasChanges(LogicItemState prev_elem_state) {
            return ProgressHasChanges(prev_elem_state);
        }
    };
} // namespace

TEST(LogicTimerSecsTestsGroup, Reference_in_limit_1_to_99999) {
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

    TestableTimerSecs testable_0;
    testable_0.SetTime(0);
    CHECK_EQUAL(1 * 1000000LL, testable_0.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_99998;
    testable_99998.SetTime(99998);
    CHECK_EQUAL(99998 * 1000000LL, testable_99998.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_99999;
    testable_99999.SetTime(99999);
    CHECK_EQUAL(99999 * 1000000LL, testable_99999.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_100000;
    testable_100000.SetTime(100000);
    CHECK_EQUAL(99999 * 1000000LL, testable_100000.PublicMorozov_GetDelayTimeUs());
}

TEST(LogicTimerSecsTestsGroup, ProgressHasChanges_true_every_one_sec) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(11, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableTimerSecs testable;
    testable.SetTime(10);
    testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive);

    CHECK_FALSE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 500000;
    CHECK_FALSE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 1000000;
    CHECK_TRUE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 1200000;
    CHECK_FALSE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 2000000;
    CHECK_TRUE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 2500000;
    CHECK_FALSE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = UINT64_MAX - 900000;
    CHECK_TRUE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 100000 - 2;
    CHECK_FALSE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));

    os_us = 100000 - 1;
    CHECK_TRUE(testable.PublicMorozov_ProgressHasChanges(LogicItemState::lisActive));
}

TEST(LogicTimerSecsTestsGroup, success_render_with_zero_progress) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableTimerSecs testable;
    testable.SetTime(10);

    uint8_t percent04 = testable.PublicMorozov_GetProgress(LogicItemState::lisActive);
    CHECK_EQUAL(0, percent04);
    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));
}

TEST(LogicTimerSecsTestsGroup, Serialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(9, writed);

    CHECK_EQUAL(TvElementType::et_TimerSecs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000000, *((uint64_t *)&buffer[1]));
}

TEST(LogicTimerSecsTestsGroup, Serialize_just_for_obtain_size) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(9, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(9, writed);
}

TEST(LogicTimerSecsTestsGroup, Serialize_to_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[1] = {};
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 123456789;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(8, readed);

    CHECK_EQUAL(123456789, testable.PublicMorozov_GetDelayTimeUs());
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_less_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 0;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_greater_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 99999 * 1000000LL + 1;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, GetElementType) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TestableTimerSecs testable;
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
}