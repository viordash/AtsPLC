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
        std::set<uint32_t> &PublicMorozov_Get_delays() {
            return delays;
        }
    };
} // namespace

TEST(ProcessTicksServiceTestsGroup, Requests_are_unique) {
    TestableProcessTicksService testable;

    testable.Request(10);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_delays().size());
    testable.Request(11);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays().size());
    testable.Request(10);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays().size());
    testable.Request(11);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_delays().size());
}
