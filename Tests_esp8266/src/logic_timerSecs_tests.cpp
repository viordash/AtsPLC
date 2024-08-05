#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/TimerSecs.cpp"
#include "main/LogicProgram/Inputs/TimerSecs.h"

TEST_GROUP(LogicTimerSecsTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

class TestableTimerSecs : public TimerSecs {
  public:
    TestableTimerSecs(uint32_t delay_time_s, InputBase &prior_item)
        : TimerSecs(delay_time_s, prior_item) {
    }
    virtual ~TestableTimerSecs() {
    }

    uint64_t PublicMorozov_GetDelayTimeUs() {
        return delay_time_us;
    }

    uint64_t PublicMorozov_GetLeftTime() {
        return GetLeftTime();
    }

    uint8_t PublicMorozov_GetProgress() {
        return GetProgress();
    }
};

TEST(LogicTimerSecsTestsGroup, Reference_in_limit_1_to_99999) {
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

    IncomeRail incomeRail0(0);
    TestableTimerSecs testable_0(0, incomeRail0);
    CHECK_EQUAL(1 * 1000000LL, testable_0.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_99998(99998, incomeRail0);
    CHECK_EQUAL(99998 * 1000000LL, testable_99998.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_99999(99999, incomeRail0);
    CHECK_EQUAL(99999 * 1000000LL, testable_99999.PublicMorozov_GetDelayTimeUs());

    TestableTimerSecs testable_100000(100000, incomeRail0);
    CHECK_EQUAL(99999 * 1000000LL, testable_100000.PublicMorozov_GetDelayTimeUs());
}

TEST(LogicTimerSecsTestsGroup, GetLeftTime_when_no_overflowed) {
    volatile uint64_t os_us = 2 * 1000000LL;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    IncomeRail incomeRail0(0);
    TestableTimerSecs testable_0(10, incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(10 * 1000000LL, left_time);

    os_us = 10 * 1000000LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(2 * 1000000LL, left_time);

    os_us = 12 * 1000000LL - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = 12 * 1000000LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = 12 * 1000000LL + 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 1LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicTimerSecsTestsGroup, GetLeftTime_when_is_overflowed) {
    volatile uint64_t os_us = UINT64_MAX - 7 * 1000000LL;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    IncomeRail incomeRail0(0);
    TestableTimerSecs testable_0(10, incomeRail0);
    uint64_t left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(10 * 1000000LL, left_time);

    os_us = UINT64_MAX - 4 * 1000000LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(7 * 1000000LL, left_time);

    os_us = UINT64_MAX;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(3 * 1000000LL, left_time);

    os_us = 0;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(3 * 1000000LL - 1, left_time);

    os_us = 3 * 1000000LL - 2;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(1, left_time);

    os_us = 3 * 1000000LL - 1;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);

    os_us = UINT64_MAX - 8 * 1000000LL;
    left_time = testable_0.PublicMorozov_GetLeftTime();
    CHECK_EQUAL(0, left_time);
}

TEST(LogicTimerSecsTestsGroup, GetProgress) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(10, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    IncomeRail incomeRail0(0);
    TestableTimerSecs testable_0(10, incomeRail0);
    uint8_t percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(0, percent);

    os_us = 1 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(1, percent);

    os_us = 5 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(5, percent);

    os_us = 10 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(10, percent);

    os_us = 25 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(25, percent);

    os_us = 50 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(50, percent);

    os_us = 99 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(99, percent);

    os_us = 100 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(100, percent);

    os_us = 120 * 100000LL;
    percent = testable_0.PublicMorozov_GetProgress();
    CHECK_EQUAL(100, percent);

}