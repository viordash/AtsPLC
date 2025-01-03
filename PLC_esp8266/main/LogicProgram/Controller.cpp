

#include "LogicProgram/Controller.h"
#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/StatusBar.h"
#include "buttons.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "hotreload_service.h"
}

static const char *TAG_Controller = "controller";

// #define STOP_PROCESS_TASK BIT1
#define STOP_RENDER_TASK BIT2
#define DO_RENDERING BIT3
#define DO_SCROLL_UP BIT4
#define DO_SCROLL_DOWN BIT5
#define DO_SELECT BIT6
#define DO_SELECT_OPTION BIT7
#define DO_SCROLL_PAGE_UP BIT8
#define DO_SCROLL_PAGE_DOWN BIT9

bool Controller::runned = NULL;
EventGroupHandle_t Controller::gpio_events = NULL;
TaskHandle_t Controller::process_task_handle = NULL;

Ladder *Controller::ladder = NULL;
ProcessWakeupService *Controller::processWakeupService = NULL;

ControllerDI Controller::DI;
ControllerAI Controller::AI;
ControllerDO Controller::O1(gpio_output::OUTPUT_0);
ControllerDO Controller::O2(gpio_output::OUTPUT_1);
ControllerVariable Controller::V1;
ControllerVariable Controller::V2;
ControllerVariable Controller::V3;
ControllerVariable Controller::V4;

void Controller::Start(EventGroupHandle_t gpio_events) {
    Controller::gpio_events = gpio_events;

    Controller::DI.Init();
    Controller::AI.Init();
    Controller::O1.Init();
    Controller::O2.Init();
    Controller::V1.Init();
    Controller::V2.Init();
    Controller::V3.Init();
    Controller::V4.Init();

    ESP_LOGI(TAG_Controller, "start");

    processWakeupService = new ProcessWakeupService();
    ladder = new Ladder([](int16_t view_top_index, int16_t selected_network) {
        hotreload->view_top_index = view_top_index;
        hotreload->selected_network = selected_network;
        store_hotreload();
    });

    Controller::runned = true;
    ESP_ERROR_CHECK(xTaskCreate(ProcessTask,
                                "ctrl_actions_task",
                                2048,
                                NULL,
                                3,
                                &Controller::process_task_handle)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void Controller::Stop() {
    Controller::runned = false;

    ESP_LOGI(TAG_Controller, "stop");
    const int tasks_stopping_timeout = 500;
    vTaskDelay(tasks_stopping_timeout / portTICK_PERIOD_MS);
    delete ladder;
    delete processWakeupService;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;

    ESP_LOGI(TAG_Controller, "start process task");

    ladder->Load();
    if (hotreload->is_hotstart) {
        ladder->SetViewTopIndex(hotreload->view_top_index);
        ladder->SetSelectedNetworkIndex(hotreload->selected_network);
    }

    TaskHandle_t render_task_handle;
    ESP_ERROR_CHECK(xTaskCreate(RenderTask,
                                "ctrl_render_task",
                                2048,
                                NULL,
                                tskIDLE_PRIORITY,
                                &render_task_handle)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);

    const uint32_t first_iteration_delay = 0;
    processWakeupService->Request((void *)Controller::ProcessTask, first_iteration_delay);
    bool need_render = true;
    while (Controller::runned) {
        EventBits_t uxBits = xEventGroupWaitBits(
            Controller::gpio_events,
            BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN
                | BUTTON_RIGHT_IO_CLOSE | BUTTON_RIGHT_IO_OPEN | BUTTON_SELECT_IO_CLOSE
                | BUTTON_SELECT_IO_OPEN | INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN,
            true,
            false,
            processWakeupService->Get());

        processWakeupService->RemoveExpired();

        bool inputs_changed = (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));
        bool buttons_changed = !inputs_changed && uxBits != 0;
        bool force_render = uxBits == 0;

        if (buttons_changed) {
            ButtonsPressType pressed_button = handle_buttons(uxBits);
            ESP_LOGD(TAG_Controller, "buttons_changed, pressed_button:%u", pressed_button);
            switch (pressed_button) {
                case ButtonsPressType::UP_PRESSED:
                    xTaskNotify(render_task_handle, DO_SCROLL_UP, eNotifyAction::eSetBits);
                    break;
                case ButtonsPressType::UP_LONG_PRESSED:
                    xTaskNotify(render_task_handle, DO_SCROLL_PAGE_UP, eNotifyAction::eSetBits);
                    break;
                case ButtonsPressType::DOWN_PRESSED:
                    xTaskNotify(render_task_handle, DO_SCROLL_DOWN, eNotifyAction::eSetBits);
                    break;
                case ButtonsPressType::SELECT_PRESSED:
                    xTaskNotify(render_task_handle, DO_SELECT, eNotifyAction::eSetBits);
                    break;
                case ButtonsPressType::SELECT_LONG_PRESSED:
                    xTaskNotify(render_task_handle, DO_SELECT_OPTION, eNotifyAction::eSetBits);
                    break;
                default:
                    break;
            }
        }

        need_render |= inputs_changed;
        need_render |= SampleIOValues();

        bool any_changes_in_actions = ladder->DoAction();
        need_render |= any_changes_in_actions;
        if (any_changes_in_actions) {
            ESP_LOGD(TAG_Controller, "any_changes_in_actions");
            Controller::RequestWakeupMs((void *)Controller::ProcessTask, 0);
        }
        need_render |= force_render;
        if (need_render) {
            need_render = false;
            xTaskNotify(render_task_handle, DO_RENDERING, eNotifyAction::eSetBits);
        }
    }

    xTaskNotify(render_task_handle, STOP_RENDER_TASK, eNotifyAction::eSetBits);
    ESP_LOGI(TAG_Controller, "stop process task");
    vTaskDelete(NULL);
}

void Controller::RenderTask(void *parm) {
    (void)parm;
    ESP_LOGI(TAG_Controller, "start render task");

    StatusBar statusBar(0);

    uint32_t ulNotifiedValue = {};

    while (Controller::runned || (ulNotifiedValue & STOP_RENDER_TASK)) {
        bool force_render = ladder->ForcePeriodicRendering();

        BaseType_t xResult =
            xTaskNotifyWait(0,
                            DO_RENDERING | DO_SCROLL_UP | DO_SCROLL_DOWN | DO_SCROLL_PAGE_UP
                                | DO_SCROLL_PAGE_DOWN | DO_SELECT | DO_SELECT_OPTION,
                            &ulNotifiedValue,
                            force_render //
                                ? 200 / portTICK_PERIOD_MS
                                : portMAX_DELAY);

        if (!force_render && xResult != pdPASS) {
            ulNotifiedValue = {};
            ESP_LOGE(TAG_Controller, "render task notify error, res:%d", xResult);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        if (ulNotifiedValue & DO_SCROLL_UP) {
            ladder->HandleButtonUp();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (ulNotifiedValue & DO_SCROLL_PAGE_UP) {
            ladder->HandleButtonPageUp();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (ulNotifiedValue & DO_SCROLL_DOWN) {
            ladder->HandleButtonDown();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (ulNotifiedValue & DO_SCROLL_PAGE_DOWN) {
            ladder->HandleButtonPageDown();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (ulNotifiedValue & DO_SELECT) {
            ladder->HandleButtonSelect();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (ulNotifiedValue & DO_SELECT_OPTION) {
            ladder->HandleButtonOption();
            ulNotifiedValue |= DO_RENDERING;
        }

        if (force_render || (ulNotifiedValue & DO_RENDERING)) {
            int64_t now_time = esp_timer_get_time();
            uint8_t *fb = begin_render();
            statusBar.Render(fb);
            ladder->Render(fb);
            end_render(fb);
            ESP_LOGD(TAG_Controller, "r (%d ms)", (int)((esp_timer_get_time() - now_time) / 1000));
        }
    }

    ESP_LOGI(TAG_Controller, "stop render task");
    vTaskDelete(NULL);
}

bool Controller::SampleIOValues() {
    bool has_changes = false;
    has_changes |= Controller::DI.SampleValue();
    has_changes |= Controller::AI.SampleValue();
    has_changes |= Controller::O1.SampleValue();
    has_changes |= Controller::O2.SampleValue();
    has_changes |= Controller::V1.SampleValue();
    has_changes |= Controller::V2.SampleValue();
    has_changes |= Controller::V3.SampleValue();
    has_changes |= Controller::V4.SampleValue();
    return has_changes;
}

bool Controller::RequestWakeupMs(void *id, uint32_t delay_ms) {
    return processWakeupService->Request(id, delay_ms);
}

void Controller::RemoveRequestWakeupMs(void *id) {
    processWakeupService->RemoveRequest(id);
}

void Controller::RemoveExpiredWakeupRequests() {
    processWakeupService->RemoveExpired();
}