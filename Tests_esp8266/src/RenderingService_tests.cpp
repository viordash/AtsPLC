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
#include "main/LogicProgram/Ladder.h"
#include "main/settings.h"

TEST_GROUP(LogicRenderingServiceTestsGroup){ //
                                             TEST_SETUP(){}

                                             TEST_TEARDOWN(){}
};

namespace {
    class TestableRenderingService : public RenderingService {
      public:
        std::atomic<bool> &PublicMorozov_Get_on_rendering() {
            return on_rendering;
        }
    };
} // namespace

TEST(LogicRenderingServiceTestsGroup, Calls_rendering_only_once_until_task_is_completed) {
    TestableRenderingService testable;

    TaskHandle_t *task_handle;
    mock()
        .expectOneCall("xTaskCreate")
        .withOutputParameterReturning("pxCreatedTask", &task_handle, sizeof(task_handle))
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", RenderingService::DO_RENDERING)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", RenderingService::STOP_RENDER_TASK)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    Ladder ladder([](int16_t view_top_index, int16_t selected_network) {
        (void)view_top_index;
        (void)selected_network;
    });
    testable.Start(&ladder);
    CHECK_FALSE(testable.PublicMorozov_Get_on_rendering());
    testable.Do();
    CHECK_TRUE(testable.PublicMorozov_Get_on_rendering());

    testable.Do();
    CHECK_TRUE(testable.PublicMorozov_Get_on_rendering());
}
