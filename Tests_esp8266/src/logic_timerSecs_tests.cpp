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
                                      TEST_SETUP(){ mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

class TestableTimerSecs : public TimerSecs {
  public:
    TestableTimerSecs(uint32_t delay_time_s, InputBase &prior_item)
        : TimerSecs(delay_time_s, prior_item) {
    }
    virtual ~TestableTimerSecs() {
    }

    uint64_t GetDelayTimeUs() {
        return delay_time_us;
    }
};

TEST(LogicTimerSecsTestsGroup, Reference_in_limit_1_to_99999) {

    IncomeRail incomeRail0(0);
    TestableTimerSecs testable_0(0, incomeRail0);
    CHECK_EQUAL(1 * 1000000LL, testable_0.GetDelayTimeUs());

    TestableTimerSecs testable_99998(99998, incomeRail0);
    CHECK_EQUAL(99998 * 1000000LL, testable_99998.GetDelayTimeUs());

    TestableTimerSecs testable_99999(99999, incomeRail0);
    CHECK_EQUAL(99999 * 1000000LL, testable_99999.GetDelayTimeUs());

    TestableTimerSecs testable_100000(100000, incomeRail0);
    CHECK_EQUAL(99999 * 1000000LL, testable_100000.GetDelayTimeUs());
}
