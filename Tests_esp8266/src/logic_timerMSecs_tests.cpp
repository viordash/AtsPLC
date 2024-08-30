#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/TimerMSecs.cpp"
#include "main/LogicProgram/Inputs/TimerMSecs.h"

TEST_GROUP(LogicTimerMSecsTestsGroup){ //
                                       TEST_SETUP(){}

                                       TEST_TEARDOWN(){}
};

namespace {
    class TestableTimerMSecs : public TimerMSecs {
      public:
        TestableTimerMSecs(uint32_t delay_time_s)
            : TimerMSecs(delay_time_s) {
        }
        virtual ~TestableTimerMSecs() {
        }

        uint64_t PublicMorozov_GetDelayTimeUs() {
            return delay_time_us;
        }
    };
} // namespace

TEST(LogicTimerMSecsTestsGroup, Reference_in_limit_1_to_99999) {
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

    
    IncomeRail incomeRail0(0, LogicItemState::lisActive);
    TestableTimerMSecs testable_0(0);
    CHECK_EQUAL(1 * 1000LL, testable_0.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_99998(99998);
    CHECK_EQUAL(99998 * 1000LL, testable_99998.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_99999(99999);
    CHECK_EQUAL(99999 * 1000LL, testable_99999.PublicMorozov_GetDelayTimeUs());

    TestableTimerMSecs testable_100000(100000);
    CHECK_EQUAL(99999 * 1000LL, testable_100000.PublicMorozov_GetDelayTimeUs());
}
