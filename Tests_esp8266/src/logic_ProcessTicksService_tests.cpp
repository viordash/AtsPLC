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
            for (const int x : delays) {
                std::cout << (first ? first = false, "" : ", ") << x;
            }
            std::cout << "]\n";
        }

        size_t PublicMorozov_Get_delays_size() {
            return std::distance(delays.begin(), delays.end());
        }
    };
} // namespace

TEST(ProcessTicksServiceTestsGroup, Requests_are_unique) {
    TestableProcessTicksService testable;

    testable.Request(10);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_delays_size());
    testable.Request(20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays_size());
    testable.Request(10);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays_size());
    testable.Request(20);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays_size());
}

TEST(ProcessTicksServiceTestsGroup,
     PopTicksToWait_returns_smallest_delay_value_and_reduce_rest_values) {
    TestableProcessTicksService testable;

    testable.Request(200);
    testable.Request(205);
    testable.Request(220);
    testable.Request(1000);
    testable.Request(2000);
    testable.Request(2005);
    testable.Request(2010);
    testable.Request(300);
    testable.Request(40);

    auto ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(4, ticksToWait);

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(20 - 4, ticksToWait);

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(22 - 20, ticksToWait);

    testable.Request(30);
    testable.println();

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(3, ticksToWait);
    testable.println();

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(30 - 22 - 3, ticksToWait);

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(100 - 30, ticksToWait);

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(200 - 100, ticksToWait);

    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(201 - 200, ticksToWait);

    const uint32_t default_delay_ticks = 10;
    ticksToWait = testable.PopTicksToWait();
    CHECK_EQUAL(default_delay_ticks, ticksToWait);
}
