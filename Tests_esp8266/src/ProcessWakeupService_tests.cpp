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
            for (auto &x : requests) {
                std::cout << (first ? first = false, "" : ", ") << x.id << "|"
                          << (x.next_time / 1000);
            }
            std::cout << "]\n";
        }

        size_t PublicMorozov_Get_requests_size() {
            return std::distance(requests.begin(), requests.end());
        }
        const ProcessWakeupRequestData &PublicMorozov_Get_request(int index) {
            auto it = std::next(requests.begin(), index);
            return *it;
        }
        const void *PublicMorozov_Get_id(int index) {
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

    testable.Request((void *)1, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)2, 20, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)1, 11, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)2, 21, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)3, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
    testable.Request((void *)4, 20, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Requests_returns_true_if_successfully_added) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    CHECK_TRUE(testable.Request((void *)1, 10, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_FALSE(testable.Request((void *)1, 11, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_TRUE(testable.Request((void *)2, 10, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_FALSE(testable.Request((void *)2, 11, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Requests_ordered_by_next_tick) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)2, 2000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)3, 100, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)1, 1000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)4, 1000, ProcessWakeupRequestPriority::pwrp_Critical);

    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)4, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(3).id);

    os_us = (uint32_t)INT32_MAX + 1000;
    testable.Request((void *)5, 50, ProcessWakeupRequestPriority::pwrp_Critical);

    CHECK_EQUAL(5, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)4, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(3).id);
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(4).id);

    testable.RemoveExpired();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());

    os_us = UINT32_MAX;
    testable.Request((void *)6, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL((void *)6, testable.PublicMorozov_Get_request(1).id);

    os_us = 999;
    testable.Request((void *)7, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL((void *)5, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL((void *)6, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL((void *)7, testable.PublicMorozov_Get_request(0).id);
}

TEST(ProcessWakeupServiceTestsGroup, Requests_ordered_by_time) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 3600000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)2, 1000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)3, 86400000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)4, 60000, ProcessWakeupRequestPriority::pwrp_Critical);

    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
    CHECK_EQUAL(100, testable.Get());
    testable.RemoveRequest((void *)2);

    CHECK_EQUAL(6000, testable.Get());
    testable.RemoveRequest((void *)4);

    CHECK_EQUAL(360000, testable.Get());
    testable.RemoveRequest((void *)1);

    CHECK_EQUAL(8640000, testable.Get());  
}

TEST(ProcessWakeupServiceTestsGroup, RemoveRequest) {
    volatile uint64_t os_us = 10000;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    testable.Request((void *)1, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)2, 20, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)3, 30, ProcessWakeupRequestPriority::pwrp_Critical);
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

    testable.Request((void *)1, 10, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)2, 20, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)3, 30, ProcessWakeupRequestPriority::pwrp_Critical);
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

    testable.Request((void *)1, 200, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)2, 1000, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)3, 300, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)4, 40, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)5, 400, ProcessWakeupRequestPriority::pwrp_Critical);

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

    testable.Request((void *)1, 215, ProcessWakeupRequestPriority::pwrp_Critical);
    auto ticksToWait = testable.Get();
    CHECK_EQUAL(22, ticksToWait);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)2, 210, ProcessWakeupRequestPriority::pwrp_Critical);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)3, 209, ProcessWakeupRequestPriority::pwrp_Critical);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)4, 205, ProcessWakeupRequestPriority::pwrp_Critical);
    ticksToWait = testable.Get();
    CHECK_EQUAL(21, ticksToWait);
    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)5, 201, ProcessWakeupRequestPriority::pwrp_Critical);
    ticksToWait = testable.Get();
    CHECK_EQUAL(20, ticksToWait);
    CHECK_EQUAL(5, testable.PublicMorozov_Get_requests_size());

    testable.Request((void *)6, 200, ProcessWakeupRequestPriority::pwrp_Critical);
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

    testable.Request(NULL, 0, ProcessWakeupRequestPriority::pwrp_Critical);
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

    testable.Request((void *)1, 100, ProcessWakeupRequestPriority::pwrp_Critical);
    testable.Request((void *)2, 200, ProcessWakeupRequestPriority::pwrp_Critical);
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

TEST(ProcessWakeupServiceTestsGroup, Idle_requests_can_be_joined_next_time_if_soon) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    CHECK_TRUE(testable.Request((void *)1, 200, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_TRUE(testable.Request((void *)2, 300, ProcessWakeupRequestPriority::pwrp_Idle));

    CHECK_TRUE(testable.Request((void *)30, 100, ProcessWakeupRequestPriority::pwrp_Idle));
    CHECK_EQUAL((void *)30, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL(100 * 1000, testable.PublicMorozov_Get_request(0).next_time);

    CHECK_TRUE(testable.Request((void *)31, 101, ProcessWakeupRequestPriority::pwrp_Idle));
    CHECK_EQUAL((void *)31, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL(200 * 1000, testable.PublicMorozov_Get_request(2).next_time);

    CHECK_TRUE(testable.Request((void *)32, 201, ProcessWakeupRequestPriority::pwrp_Idle));
    CHECK_EQUAL((void *)32, testable.PublicMorozov_Get_request(4).id);
    CHECK_EQUAL(300 * 1000, testable.PublicMorozov_Get_request(4).next_time);

    os_us = 110 * 1000;
    testable.RemoveExpired();
    CHECK_EQUAL(4, testable.PublicMorozov_Get_requests_size());
    os_us = 200 * 1000;
    testable.RemoveExpired();
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests_size());
}

TEST(ProcessWakeupServiceTestsGroup, Critical_requests_cannot_be_joined) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(4, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableProcessWakeupService testable;

    CHECK_TRUE(testable.Request((void *)1, 200, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_TRUE(testable.Request((void *)2, 300, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_TRUE(testable.Request((void *)3, 250, ProcessWakeupRequestPriority::pwrp_Critical));
    CHECK_TRUE(testable.Request((void *)4, 150, ProcessWakeupRequestPriority::pwrp_Critical));

    CHECK_EQUAL((void *)4, testable.PublicMorozov_Get_request(0).id);
    CHECK_EQUAL(150 * 1000, testable.PublicMorozov_Get_request(0).next_time);

    CHECK_EQUAL((void *)1, testable.PublicMorozov_Get_request(1).id);
    CHECK_EQUAL(200 * 1000, testable.PublicMorozov_Get_request(1).next_time);

    CHECK_EQUAL((void *)3, testable.PublicMorozov_Get_request(2).id);
    CHECK_EQUAL(250 * 1000, testable.PublicMorozov_Get_request(2).next_time);

    CHECK_EQUAL((void *)2, testable.PublicMorozov_Get_request(3).id);
    CHECK_EQUAL(300 * 1000, testable.PublicMorozov_Get_request(3).next_time);
}