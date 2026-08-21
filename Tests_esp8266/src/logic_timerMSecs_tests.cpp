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
        const char *PublicMorozov_Get_str_time() {
            return str_time;
        }
    };
} // namespace

TEST(LogicTimerMSecsTestsGroup, Time_in_limit_50_to_99950) {
    TimerMSecs testable_0;
    testable_0.SetTime(0);
    CHECK_EQUAL(50 * 1000LL, testable_0.GetTimeUs());

    TimerMSecs testable_99900;
    testable_99900.SetTime(99900);
    CHECK_EQUAL(99900 * 1000LL, testable_99900.GetTimeUs());

    TimerMSecs testable_99950;
    testable_99950.SetTime(99950);
    CHECK_EQUAL(99950 * 1000LL, testable_99950.GetTimeUs());

    TimerMSecs testable_99951;
    testable_99951.SetTime(99951);
    CHECK_EQUAL(99950 * 1000LL, testable_99951.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(9, writed);

    CHECK_EQUAL(TvElementType::et_TimerMSecs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000, *((uint64_t *)&buffer[1]));
}

TEST(LogicTimerMSecsTestsGroup, Serialize_just_for_obtain_size) {
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(9, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(9, writed);
}

TEST(LogicTimerMSecsTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TimerMSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 123456;

    TestableTimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(8, readed);

    CHECK_EQUAL(123000, testable.GetTimeUs());
    STRCMP_EQUAL("123", testable.PublicMorozov_Get_str_time());
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_less_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 0;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, Deserialize_with_greater_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[1]) = 99950 * 1000LL + 1;

    TimerMSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerMSecsTestsGroup, GetElementType) {
    TimerMSecs testable;
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
}

TEST(LogicTimerMSecsTestsGroup, TryToCast) {
    TimerMSecs timerMSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerMSecs) == &timerMSecs);

    TimerSecs timerSecs;
    CHECK_TRUE(TimerMSecs::TryToCast(&timerSecs) == NULL);
}

TEST(LogicTimerMSecsTestsGroup, SelectPrior_changing_delay_time) {
    TimerMSecs testable(1);
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(100 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(150 * 1000L, testable.GetTimeUs());

    testable.SetTime(99900);
    testable.SelectPrior();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, SelectNext_changing_IoAdr) {
    TimerMSecs testable(150);
    testable.BeginEditing();
    testable.SelectNext();
    CHECK_EQUAL(100 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(50 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(50 * 1000L, testable.GetTimeUs());

    testable.SetTime(99950);
    testable.SelectNext();
    CHECK_EQUAL(99900 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99850 * 1000L, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99800 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, PageUp_changing_delay_time) {
    TimerMSecs testable(1);
    testable.BeginEditing();
    testable.PageUp();
    CHECK_EQUAL(300 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(550 * 1000L, testable.GetTimeUs());

    testable.SetTime(99700);
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetTimeUs());
}

TEST(LogicTimerMSecsTestsGroup, PageDown_changing_IoAdr) {
    TimerMSecs testable(350);
    testable.BeginEditing();
    testable.PageDown();
    CHECK_EQUAL(100 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(50 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(50 * 1000L, testable.GetTimeUs());

    testable.SetTime(99950);
    testable.PageDown();
    CHECK_EQUAL(99700 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99450 * 1000L, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99200 * 1000L, testable.GetTimeUs());
}