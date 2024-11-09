#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <set>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ProcessWakeupService.cpp"

TEST_GROUP(ProcessWakeupServiceTestsGroup){ //
                                            TEST_SETUP(){}

                                            TEST_TEARDOWN(){}
};

namespace {

    class TestableProcessWakeupService : public ProcessWakeupService {
      public:
        void println() {
            std::cout << '[';
            bool first{ true };
            for (auto x : requests) {
                std::cout << (first ? first = false, "" : ", ") << x.id << "|" << x.next_tick;
            }
            std::cout << "]\n";
        }

        size_t PublicMorozov_Get_ticks_size() {
            return std::distance(requests.begin(), requests.end());
        }
        int32_t PublicMorozov_GetTimespan(uint32_t from, uint32_t to) {
            return GetTimespan(from, to);
        }
    };
} // namespace

TEST(ProcessWakeupServiceTestsGroup, Requests_are_unique_in_range_of_portTICK_PERIOD_MS) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(9, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)2, 20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)3, 10);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)4, 20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)5, 21);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)6, 25);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)7, 28);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)8, 29);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
    testable.Request((void *)9, 30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
}

TEST(ProcessWakeupServiceTestsGroup, Requests_remove_expired_ticks) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(6, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10);
    testable.Request((void *)2, 20);
    testable.Request((void *)3, 30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
    ticks += 3;

    testable.Request((void *)4, 40);
    testable.Request((void *)5, 50);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());

    testable.println();
    ticks = INT32_MAX;
    testable.Request((void *)6, 0);
    testable.println();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_ticks_size());
}

TEST(ProcessWakeupServiceTestsGroup, Get_returns_early_tick) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(6, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 200);
    testable.Request((void *)2, 1000);
    testable.Request((void *)3, 300);
    testable.Request((void *)4, 40);
    testable.Request((void *)5, 400);

    auto ticksToWait = testable.Get();
    CHECK_EQUAL(4, ticksToWait);
}

TEST(ProcessWakeupServiceTestsGroup, Get_returns_default_when_empty) {
    mock().expectNoCall("xTaskGetTickCount");

    TestableProcessWakeupService testable;

    auto ticksToWait = testable.Get();
    const uint32_t default_delay_ticks = -1;
    CHECK_EQUAL(default_delay_ticks, ticksToWait);
}

TEST(ProcessWakeupServiceTestsGroup, Requested_ticks_rounds_to_up) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(12, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 211);
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(22, ticksToWait);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_ticks_size());

    testable.Request((void *)2, 210);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());

    testable.Request((void *)3, 209);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());

    testable.Request((void *)4, 205);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());

    testable.Request((void *)5, 201);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());

    testable.Request((void *)6, 200);
    ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_ticks_size());
}

TEST(ProcessWakeupServiceTestsGroup, Request_zero) {
    volatile uint32_t ticks = 51615;
    mock()
        .expectNCalls(2, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request(NULL, 0);
    ticks = 51616;
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(0, ticksToWait);
}

TEST(ProcessWakeupServiceTestsGroup, RemoveExpired) {
    volatile uint32_t ticks = 10000;
    mock()
        .expectNCalls(5, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 100);
    testable.Request((void *)2, 200);
    testable.println();
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());

    ticks += 5;
    testable.RemoveExpired();
    CHECK_EQUAL(2, testable.PublicMorozov_Get_ticks_size());
    testable.println();

    ticks += 5;
    testable.RemoveExpired();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_ticks_size());

    ticks += 100;
    testable.RemoveExpired();
    CHECK_EQUAL(0, testable.PublicMorozov_Get_ticks_size());
}

TEST(ProcessWakeupServiceTestsGroup, GetTimespan) {
    TestableProcessWakeupService testable;

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