#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "LogicProgram/LogicElement.h"
#include "main/Display/RenderingService.h"
#include "main/LogicProgram/Controller.h"
#include "main/settings.h"

TEST_GROUP(LogicRenderingServiceTestsGroup){ //
                                             TEST_SETUP(){}

                                             TEST_TEARDOWN(){}
};

namespace {
    class TestableRenderingService : public RenderingService {
      public:
        const uint32_t min_period_ticks = 15;
        void PublicMorozov_UpdateLoopTime() {
            UpdateLoopTime();
        }
        bool PublicMorozov_Skip(uint32_t next_awake_time_interval) {
            return Skip(next_awake_time_interval);
        }
    };
} // namespace

TEST(LogicRenderingServiceTestsGroup, Skip_rendering_when_time_interval_is_too_short) {
    TestableRenderingService testable;

    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(6, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    testable.PublicMorozov_UpdateLoopTime();
    CHECK_TRUE(testable.PublicMorozov_Skip(testable.min_period_ticks - 1));

    os_us = (testable.min_period_ticks * portTICK_PERIOD_MS * 1000) + 1;
    CHECK_FALSE(testable.PublicMorozov_Skip(testable.min_period_ticks - 1));

    os_us = __UINT64_MAX__;
    testable.PublicMorozov_UpdateLoopTime();
    os_us = (testable.min_period_ticks * portTICK_PERIOD_MS * 1000) - 2;
    CHECK_TRUE(testable.PublicMorozov_Skip(testable.min_period_ticks - 1));

    os_us = (testable.min_period_ticks * portTICK_PERIOD_MS * 1000) - 1;
    CHECK_FALSE(testable.PublicMorozov_Skip(testable.min_period_ticks - 1));
}

TEST(LogicRenderingServiceTestsGroup, Dont_skip_rendering_when_next_time_is_far) {
    TestableRenderingService testable;
    CHECK_FALSE(testable.PublicMorozov_Skip(testable.min_period_ticks));

    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    testable.PublicMorozov_UpdateLoopTime();
    CHECK_TRUE(testable.PublicMorozov_Skip(testable.min_period_ticks - 1));
}
