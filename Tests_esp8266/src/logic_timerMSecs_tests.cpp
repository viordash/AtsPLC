#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/TimerMSecs.cpp"
#include "main/LogicProgram/Inputs/TimerMSecs.h"
#include "main/LogicProgram/Inputs/TimerSecs.h"

TEST_GROUP(LogicTimerMSecsTestsGroup){ //
                                       TEST_SETUP(){}

                                       TEST_TEARDOWN(){}
};

TEST(LogicTimerMSecsTestsGroup, Reference_in_limit_1_to_99999) {
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

    TimerMSecs testable_0;
    testable_0.SetTime(0);
    CHECK_EQUAL(1 * 1000LL, testable_0.GetTimeUs());

    TimerMSecs testable_99998;
    testable_99998.SetTime(99998);
    CHECK_EQUAL(99998 * 1000LL, testable_99998.GetTimeUs());

    TimerMSecs testable_99999;
    testable_99999.SetTime(99999);
    CHECK_EQUAL(99999 * 1000LL, testable_99999.GetTimeUs());

    TimerMSecs testable_100000;
    testable_100000.SetTime(100000);
    CHECK_EQUAL(99999 * 1000LL, testable_100000.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, Serialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(9, writed);

    CHECK_EQUAL(TvElementType::et_TimerMSecs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000, *((uint64_t *)&buffer[1]));
}

TEST(LogicTimerMSecsTestsGroup, Serialize_just_for_obtain_size) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(9, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(9, writed);
}

TEST(LogicTimerMSecsTestsGroup, Serialize_to_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[1] = {};
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 123456789;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(8, readed);

    CHECK_EQUAL(123456789, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_less_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 0;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_greater_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 99999 * 1000000LL + 1;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, GetElementType) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable;
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
}

TEST(LogicTimerMSecsTestsGroup, TryToCast) {
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();

    TimerMSecs timerMSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerMSecs) == &timerMSecs);

    TimerSecs timerSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerSecs) == NULL);
}

TEST(LogicTimerMSecsTestsGroup, SelectPrior_changing_delay_time) {
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable(1);
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(51 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(101 * 1000L, testable.GetTimeUs());

    testable.SetTime(99900);
    testable.SelectPrior();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99999 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99999 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, SelectNext_changing_IoAdr) {
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable(150);
    testable.BeginEditing();
    testable.SelectNext();
    CHECK_EQUAL(100 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(50 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(1 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(1 * 1000L, testable.GetTimeUs());

    testable.SetTime(99999);
    testable.SelectNext();
    CHECK_EQUAL(99949 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99899 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99849 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, PageUp_changing_delay_time) {
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable(1);
    testable.BeginEditing();
    testable.PageUp();
    CHECK_EQUAL(251 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(501 * 1000L, testable.GetTimeUs());

    testable.SetTime(99700);
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99999 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99999 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, PageDown_changing_IoAdr) {
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    TimerMSecs testable(350);
    testable.BeginEditing();
    testable.PageDown();
    CHECK_EQUAL(100 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(1 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(1 * 1000L, testable.GetTimeUs());

    testable.SetTime(99999);
    testable.PageDown();
    CHECK_EQUAL(99749 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99499 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99249 * 1000L, testable.GetTimeUs());
}