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
        TaskHandle_t *PublicMorozov_Get_task_handle() {
            return &task_handle;
        }
        TaskArg *PublicMorozov_Get_task_arg() {
            return &task_arg;
        }
        void PublicMorozov_Task() {
            RenderingService::Task(&this->task_arg);
        }
    };
} // namespace

TEST(LogicRenderingServiceTestsGroup, Do_calls_xTaskNotify_with_DO_RENDERING) {
    TestableRenderingService testable;

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withPointerParameter("xTaskToNotify", (TaskHandle_t)42)
        .withUnsignedIntParameter("ulValue", RenderingService::DO_RENDERING)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", RenderingService::STOP_RENDER_TASK)
        .ignoreOtherParameters();

    *testable.PublicMorozov_Get_task_handle() = (TaskHandle_t)42;
    testable.Do();
}

TEST(LogicRenderingServiceTestsGroup, Rendering_task_DO_RENDERING) {
    TestableRenderingService testable;

    TaskHandle_t currentTaskHandle = (TaskHandle_t)42;
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    mock().expectNCalls(1, "xTaskGetCurrentTaskHandle").andReturnValue(currentTaskHandle);

    uint32_t render_notify = RenderingService::DO_RENDERING;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withOutputParameterReturning("pulNotificationValue", &render_notify, sizeof(render_notify))
        .ignoreOtherParameters();

    uint32_t stop_notify = RenderingService::STOP_RENDER_TASK;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withOutputParameterReturning("pulNotificationValue", &stop_notify, sizeof(stop_notify))
        .ignoreOtherParameters();

    mock().expectOneCall("vTaskDelete").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withPointerParameter("xTaskToNotify", currentTaskHandle)
        .withUnsignedIntParameter("ulValue", RenderingService::DO_RENDERING)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", RenderingService::STOP_RENDER_TASK)
        .ignoreOtherParameters();

    Ladder ladder;
    ;
    *testable.PublicMorozov_Get_task_arg() = { &testable, &ladder };
    *testable.PublicMorozov_Get_task_handle() = (TaskHandle_t)42;

    testable.PublicMorozov_Task();
}
