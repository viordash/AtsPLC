#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/TimerMSecs.h"
#include "main/LogicProgram/Inputs/TimerSecs.cpp"
#include "main/LogicProgram/Inputs/TimerSecs.h"

static uint8_t frame_buffer[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH] = {};

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
        const char *PublicMorozov_Get_str_time() {
            return str_time;
        }
    };
} // namespace

TEST(LogicTimerSecsTestsGroup, Time_in_limit_1_to_99999) {
    TestableTimerSecs testable_0;
    testable_0.SetTime(0);
    CHECK_EQUAL(1 * 1000000LL, testable_0.GetTimeUs());

    TestableTimerSecs testable_99998;
    testable_99998.SetTime(99998);
    CHECK_EQUAL(99998 * 1000000LL, testable_99998.GetTimeUs());

    TestableTimerSecs testable_99999;
    testable_99999.SetTime(99999);
    CHECK_EQUAL(99999 * 1000000LL, testable_99999.GetTimeUs());

    TestableTimerSecs testable_100000;
    testable_100000.SetTime(100000);
    CHECK_EQUAL(99999 * 1000000LL, testable_100000.GetTimeUs());
}

TEST(LogicTimerSecsTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(9, writed);

    CHECK_EQUAL(TvElementType::et_TimerSecs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000000, *((uint64_t *)&buffer[1]));
}

TEST(LogicTimerSecsTestsGroup, Serialize_just_for_obtain_size) {
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(9, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(9, writed);
}

TEST(LogicTimerSecsTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableTimerSecs testable;
    testable.SetTime(12345);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 123456789;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(8, readed);

    CHECK_EQUAL(123000000, testable.GetTimeUs());
    STRCMP_EQUAL("123", testable.PublicMorozov_Get_str_time());
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_less_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 0;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, Deserialize_with_greater_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_TimerSecs;
    *((uint64_t *)&buffer[1]) = 99999 * 1000000LL + 1;

    TestableTimerSecs testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicTimerSecsTestsGroup, GetElementType) {
    TestableTimerSecs testable;
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
}

TEST(LogicTimerSecsTestsGroup, TryToCast) {

    TimerMSecs timerMSecs;
    CHECK_TRUE(TimerSecs::TryToCast(&timerMSecs) == NULL);

    TimerSecs timerSecs;
    CHECK_TRUE(TimerSecs::TryToCast(&timerSecs) == &timerSecs);
}

TEST(LogicTimerSecsTestsGroup, SelectPrior_changing_delay_time) {
    TimerSecs testable(1);
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(2 * 1000000LL, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(3 * 1000000LL, testable.GetTimeUs());

    testable.SetTime(99997);
    testable.SelectPrior();
    CHECK_EQUAL(99998 * 1000000LL, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99999 * 1000000LL, testable.GetTimeUs());
    testable.SelectPrior();
    CHECK_EQUAL(99999 * 1000000LL, testable.GetTimeUs());
}

TEST(LogicTimerSecsTestsGroup, SelectNext_changing_IoAdr) {
    TimerSecs testable(3);
    testable.BeginEditing();
    testable.SelectNext();
    CHECK_EQUAL(2 * 1000000LL, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(1 * 1000000LL, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(1 * 1000000LL, testable.GetTimeUs());

    testable.SetTime(99999);
    testable.SelectNext();
    CHECK_EQUAL(99998 * 1000000LL, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99997 * 1000000LL, testable.GetTimeUs());
    testable.SelectNext();
    CHECK_EQUAL(99996 * 1000000LL, testable.GetTimeUs());
}

TEST(LogicTimerSecsTestsGroup, PageUp_changing_delay_time) {
    TimerSecs testable(1);
    testable.BeginEditing();
    testable.PageUp();
    CHECK_EQUAL(11 * 1000000LL, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(21 * 1000000LL, testable.GetTimeUs());

    testable.SetTime(99987);
    testable.PageUp();
    CHECK_EQUAL(99997 * 1000000LL, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99999 * 1000000LL, testable.GetTimeUs());
    testable.PageUp();
    CHECK_EQUAL(99999 * 1000000LL, testable.GetTimeUs());
}

TEST(LogicTimerSecsTestsGroup, PageDown_changing_IoAdr) {
    TimerSecs testable(15);
    testable.BeginEditing();
    testable.PageDown();
    CHECK_EQUAL(5 * 1000000LL, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(1 * 1000000LL, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(1 * 1000000LL, testable.GetTimeUs());

    testable.SetTime(99999);
    testable.PageDown();
    CHECK_EQUAL(99989 * 1000000LL, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99979 * 1000000LL, testable.GetTimeUs());
    testable.PageDown();
    CHECK_EQUAL(99969 * 1000000LL, testable.GetTimeUs());
}