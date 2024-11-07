#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <set>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ProcessTicksService.cpp"

TEST_GROUP(ProcessTicksServiceTestsGroup){ //
                                           TEST_SETUP(){}

                                           TEST_TEARDOWN(){}
};

namespace {

    class TestableProcessTicksService : public ProcessTicksService {
      public:
        void println() {
            std::cout << '[';
            bool first{ true };
            for (auto x : moments) {
                std::cout << (first ? first = false, "" : ", ") << x;
            }
            std::cout << "]\n";
        }

        size_t PublicMorozov_Get_moments_size() {
            return std::distance(moments.begin(), moments.end());
        }
        int32_t PublicMorozov_GetTimespan(uint32_t from, uint32_t to) {
            return GetTimespan(from, to);
        }
    };
} // namespace

TEST(ProcessTicksServiceTestsGroup, Requests_are_unique_in_range_of_portTICK_PERIOD_MS) {
    volatile uint64_t os_us = 100000;
    mock()
        .expectNCalls(9, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(10);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_moments_size());
    testable.Request(20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_moments_size());
    testable.Request(10);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_moments_size());
    testable.Request(20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_moments_size());

    testable.println();
    testable.Request(21);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
    testable.Request(25);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
    testable.Request(28);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
    testable.Request(29);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
    testable.Request(30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
}

TEST(ProcessTicksServiceTestsGroup, Requests_remove_expired_ticks) {
    volatile uint64_t os_us = 100000;
    mock()
        .expectNCalls(6, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(10);
    testable.Request(20);
    testable.Request(30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());
    os_us += 30 * 1000;

    testable.Request(40);
    testable.Request(50);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_moments_size());

    testable.println();
    os_us = (int64_t)INT32_MAX * 1000LL;
    testable.Request(0);
    testable.println();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_moments_size());
}

TEST(ProcessTicksServiceTestsGroup, Get_returns_early_tick_or_default) {
    volatile uint64_t os_us = 100000;
    mock()
        .expectNCalls(18, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(200);
    testable.Request(220);
    testable.Request(1000);
    testable.Request(2000);
    testable.Request(2010);
    testable.Request(300);
    testable.Request(40);

    auto ticksToWait = testable.Get();
    CHECK_EQUAL(4, ticksToWait);

    testable.Request(0);
    ticksToWait = testable.Get();
    CHECK_EQUAL(0, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(22, ticksToWait);

    testable.Request(30);

    ticksToWait = testable.Get();
    CHECK_EQUAL(3, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(30, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(100, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(200, ticksToWait);

    ticksToWait = testable.Get();
    CHECK_EQUAL(201, ticksToWait);

    const uint32_t default_delay_ticks = -1;
    ticksToWait = testable.Get();
    CHECK_EQUAL(default_delay_ticks, ticksToWait);
}

TEST(ProcessTicksServiceTestsGroup, Request_zero) {
    volatile uint64_t os_us = 516159000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(0);
    os_us = 516163000;
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(0, ticksToWait);
}

TEST(ProcessTicksServiceTestsGroup, Requested_ticks_rounds_to_up) {
    volatile uint64_t os_us = 100000;
    mock()
        .expectNCalls(12, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(200);
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);

    testable.Request(201);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);

    testable.Request(205);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);

    testable.Request(209);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);

    testable.Request(210);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);

    testable.Request(211);
    ticksToWait = testable.Get();
    CHECK_EQUAL(22, ticksToWait);
}

TEST(ProcessTicksServiceTestsGroup, Get_skips_expired_ticks) {
    volatile uint64_t os_us = 100000;
    mock()
        .expectNCalls(11, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessTicksService testable;

    testable.Request(100);
    testable.Request(200);
    testable.Request(300);
    testable.Request(400);
    testable.Request(500);
    testable.Request(600);
    testable.Request(700);
    testable.println();

    auto ticksToWait = testable.Get();
    CHECK_EQUAL(10, ticksToWait);

    os_us += 50 * 1000;

    ticksToWait = testable.Get();
    CHECK_EQUAL(15, ticksToWait);

    os_us += 350 * 1000;

    ticksToWait = testable.Get();
    CHECK_EQUAL(0, ticksToWait);

    os_us += 1000 * 1000;

    const uint32_t default_delay_ticks = -1;
    ticksToWait = testable.Get();
    CHECK_EQUAL(default_delay_ticks, ticksToWait);
}

TEST(ProcessTicksServiceTestsGroup, GetTimespan) {
    TestableProcessTicksService testable;

    auto timespan = testable.PublicMorozov_GetTimespan(100, 400);
    CHECK_EQUAL(300, timespan);

    timespan = testable.PublicMorozov_GetTimespan(400, 100);
    CHECK_EQUAL(-300, timespan);

    timespan = testable.PublicMorozov_GetTimespan(UINT32_MAX - 100, 0);
    CHECK_EQUAL(101, timespan);

    timespan = testable.PublicMorozov_GetTimespan(0, UINT32_MAX - 100);
    CHECK_EQUAL(-101, timespan);

    timespan = testable.PublicMorozov_GetTimespan(UINT32_MAX - 100, UINT32_MAX);
    CHECK_EQUAL(100, timespan);

    timespan = testable.PublicMorozov_GetTimespan(UINT32_MAX, UINT32_MAX - 100);
    CHECK_EQUAL(-100, timespan);

    timespan = testable.PublicMorozov_GetTimespan(0, INT32_MAX);
    CHECK_EQUAL(INT32_MAX, timespan);

    timespan = testable.PublicMorozov_GetTimespan(INT32_MAX, 0);
    CHECK_EQUAL(-INT32_MAX, timespan);

    timespan = testable.PublicMorozov_GetTimespan(INT32_MAX - 100, UINT32_MAX - 101);
    CHECK_EQUAL(INT32_MAX, timespan);

    timespan = testable.PublicMorozov_GetTimespan(UINT32_MAX - 101, INT32_MAX - 100);
    CHECK_EQUAL(-INT32_MAX, timespan);

    timespan = testable.PublicMorozov_GetTimespan(UINT32_MAX, 0);
    CHECK_EQUAL(1, timespan);

    timespan = testable.PublicMorozov_GetTimespan(0, UINT32_MAX);
    CHECK_EQUAL(-1, timespan);
}

// TEST(ProcessTicksServiceTestsGroup, Get_test) {
//     volatile uint64_t os_us = 241693 * 1000;
//     mock()
//         .expectNCalls(16, "esp_timer_get_time")
//         .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

//     TestableProcessTicksService testable;

//     testable.Request(1000);

//     os_us = 241711 * 1000;
//     auto ticksToWait = testable.Get();
//     CHECK_EQUAL(98, ticksToWait);
// }