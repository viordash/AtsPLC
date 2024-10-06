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

namespace {
    class TestableTimerMSecs : public TimerMSecs {
      public:
        TestableTimerMSecs() : TimerMSecs() {
        }
        virtual ~TestableTimerMSecs() {
        }

        uint64_t PublicMorozov_GetDelayTimeUs() {
            return delay_time_us;
        }
    };
} // namespace

TEST(LogicTimerMSecsTestsGroup, Reference_in_limit_1_to_99999) {
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

    TestableTimerMSecs testable_0;
    testable_0.SetTime(0);
    CHECK_EQUAL(1 * 1000LL, testable_0.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_99998;
    testable_99998.SetTime(99998);
    CHECK_EQUAL(99998 * 1000LL, testable_99998.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_99999;
    testable_99999.SetTime(99999);
    CHECK_EQUAL(99999 * 1000LL, testable_99999.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_100000;
    testable_100000.SetTime(100000);
    CHECK_EQUAL(99999 * 1000LL, testable_100000.PublicMorozov_GetDelayTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, Serialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    TestableTimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(9, writed);

    CHECK_EQUAL(TvElementType::et_TimerMSecs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000, *((uint64_t *)&buffer[1]));
}

TEST(LogicTimerMSecsTestsGroup, Serialize_just_for_obtain_size) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TestableTimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(9, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(9, writed);
}

TEST(LogicTimerMSecsTestsGroup, Serialize_to_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[1] = {};
    TestableTimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 123456789;

    TestableTimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(8, readed);

    CHECK_EQUAL(123456789, testable.PublicMorozov_GetDelayTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_small_buffer_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;

    TestableTimerMSecs testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_less_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 0;

    TestableTimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_greater_value_return_zero) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 99999 * 1000000LL + 1;

    TestableTimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, GetElementType) {
    mock().expectOneCall("esp_timer_get_time").ignoreOtherParameters();
    TestableTimerMSecs testable;
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
}

TEST(LogicTimerMSecsTestsGroup, TryToCast) {
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();

    TimerMSecs timerMSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerMSecs) == &timerMSecs);

    TimerSecs timerSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerSecs) == NULL);
}