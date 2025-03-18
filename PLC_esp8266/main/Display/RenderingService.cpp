#include "Display/RenderingService.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/StatusBar.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_RenderingService = "RenderingService";

RenderingService::RenderingService() {
    task_handle = NULL;
    task_arg = {};
}

RenderingService::~RenderingService() {
    Stop();
}

void RenderingService::Task(void *parm) {
    ESP_LOGI(TAG_RenderingService, "start task");

    TaskArg *arg = static_cast<TaskArg *>(parm);

    StatusBar statusBar(0);
    uint32_t ulNotifiedValue = {};

    while ((ulNotifiedValue & STOP_RENDER_TASK) == 0) {
        arg->service->on_rendering = false;
        BaseType_t xResult =
            xTaskNotifyWait(0, DO_RENDERING | STOP_RENDER_TASK, &ulNotifiedValue, portMAX_DELAY);

        if (xResult != pdPASS) {
            ulNotifiedValue = {};
            ESP_LOGE(TAG_RenderingService, "task notify error, res:%d", xResult);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        if (ulNotifiedValue & DO_RENDERING) {
            int64_t time_before_render = esp_timer_get_time();
            uint8_t *fb = begin_render();
            statusBar.Render(fb);
            arg->ladder->Render(fb);
            end_render(fb);

            int64_t time_after_render = esp_timer_get_time();
            static int64_t loop_time = 0;
            ESP_LOGD(TAG_RenderingService,
                     "r %d ms (%d ms)",
                     (int)((time_after_render - loop_time) / 1000),
                     (int)((time_after_render - time_before_render) / 1000));
            loop_time = time_after_render;
        }
    }
    arg->service->on_rendering = false;
    ESP_LOGI(TAG_RenderingService, "stop task");
    vTaskDelete(NULL);
}

void RenderingService::Start(Ladder *ladder) {
    if (task_handle != NULL) {
        return;
    }
    on_rendering = false;

    task_arg = { this, ladder };
    ESP_ERROR_CHECK(
        xTaskCreate(Task, "ctrl_render_task", 4096, &task_arg, tskIDLE_PRIORITY, &task_handle)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);
}

void RenderingService::Stop() {
    if (task_handle == NULL) {
        return;
    }
    xTaskNotify(task_handle, STOP_RENDER_TASK, eNotifyAction::eSetBits);
    task_handle = NULL;
}

void RenderingService::Do() {
    if (task_handle == NULL) {
        return;
    }
    if (on_rendering) {
        ESP_LOGD(TAG_RenderingService, "do skip");
        return;
    }
    ESP_LOGD(TAG_RenderingService, "do");
    on_rendering = true;
    xTaskNotify(task_handle, DO_RENDERING, eNotifyAction::eSetBits);
}