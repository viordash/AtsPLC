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
                std::cout << (first ? first = false, "" : ", ") << x.id << "|"
                          << (x.next_time / 1000);
            }
            std::cout << "]\n";
        }

        size_t PublicMorozov_Get_requests_size() {
            return std::distance(requests.begin(), requests.end());
        }
        ProcessWakeupRequestData PublicMorozov_Get_request(int index) {
            auto it = std::next(requests.begin(), index);
            return *it;
        }
        void *PublicMorozov_Get_id(int index) {
            auto it = std::next(ids.begin(), index);
            return *it;
        }
    };
} // namespace

TEST(ProcessWakeupServiceTestsGroup, Requests_are_unique_by_id) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(4, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)2, 20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)1, 11);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)2, 21);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)3, 10);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)4, 20);
    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Requests_returns_true_if_successfully_added) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    CHECK_TRUE(testable.Request((void *)1, 10));
    CHECK_FALSE(testable.Request((void *)1, 11));
    CHECK_TRUE(testable.Request((void *)2, 10));
    CHECK_FALSE(testable.Request((void *)2, 11));
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Requests_ordered_by_next_tick) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)2, 2000);
    testable.Request((void *)3, 100);
    testable.Request((void *)1, 1000);
    testable.Request((void *)4, 1000);

    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)4, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(3).id);

    os_us = (uint32_t)INT32_MAX + 1000;
    testable.Request((void *)5, 50);

    CHECK_EQUAL(5, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)4, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(3).id);
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(4).id);

    testable.RemoveExpired();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());

    os_us = UINT32_MAX;
    testable.Request((void *)6, 10);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)6, testable.PublicMorozov_Get_request(1).id);

    os_us = 999;
    testable.Request((void *)7, 10);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)6, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)7, testable.PublicMorozov_Get_request(2).id);
}

TEST(ProcessWakeupServiceTestsGroup, RemoveRequest) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10);
    testable.Request((void *)2, 20);
    testable.Request((void *)3, 30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(2).id);

    testable.RemoveRequest((void *)3);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(1).id);

    testable.RemoveRequest((void *)2);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(0).id);

    testable.RemoveRequest((void *)1);
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Remove_not_exists_request) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10);
    testable.Request((void *)2, 20);
    testable.Request((void *)3, 30);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());

    testable.RemoveRequest((void *)4);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());

    testable.RemoveRequest((void *)5);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Get_returns_early_tick) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(6, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

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
    mock().expectNoCall("esp_timer_get_time");

    TestableProcessWakeupService testable;

    auto ticksToWait = testable.Get();
    const uint32_t default_delay_ticks = -1;
    CHECK_EQUAL(default_delay_ticks, ticksToWait);
}

TEST(ProcessWakeupServiceTestsGroup, Requested_round_ticks_to_up) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(12, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 215);
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(22, ticksToWait);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)2, 210);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)3, 209);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)4, 205);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)5, 201);
    ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);
    CHECK_EQUAL(5, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)6, 200);
    ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);
    CHECK_EQUAL(6, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Request_zero) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request(NULL, 0);
    os_us = 51616;
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(0, ticksToWait);
}

TEST(ProcessWakeupServiceTestsGroup, RemoveExpired) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(5, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 100);
    testable.Request((void *)2, 200);
    testable.println();
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());

    os_us += 50 * 1000;
    testable.RemoveExpired();
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    testable.println();

    os_us += 50 * 1000;
    testable.RemoveExpired();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());

    os_us += 1000 * 1000;
    testable.RemoveExpired();
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, GetTimespan) {
    TestableProcessWakeupService testable;

    auto timespan = ProcessWakeupRequestDataCmp::GetTimespan(100, 400);
    CHECK_EQUAL(300, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(400, 100);
    CHECK_EQUAL(-300, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(UINT64_MAX - 100, 0);
    CHECK_EQUAL(101, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(0, UINT64_MAX - 100);
    CHECK_EQUAL(-101, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(UINT64_MAX - 100, UINT64_MAX);
    CHECK_EQUAL(100, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(UINT64_MAX, UINT64_MAX - 100);
    CHECK_EQUAL(-100, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(0, INT64_MAX);
    CHECK_EQUAL(INT64_MAX, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(INT64_MAX, 0);
    CHECK_EQUAL(-INT64_MAX, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(INT64_MAX - 100, UINT64_MAX - 101);
    CHECK_EQUAL(INT64_MAX, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(UINT64_MAX - 101, INT64_MAX - 100);
    CHECK_EQUAL(-INT64_MAX, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(UINT64_MAX, 0);
    CHECK_EQUAL(1, timespan);

    timespan = ProcessWakeupRequestDataCmp::GetTimespan(0, UINT64_MAX);
    CHECK_EQUAL(-1, timespan);
}