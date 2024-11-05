

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

uint8_t Controller::var1 = LogicElement::MinValue;
uint8_t Controller::var2 = LogicElement::MinValue;
uint8_t Controller::var3 = LogicElement::MinValue;
uint8_t Controller::var4 = LogicElement::MinValue;

Ladder *Controller::ladder = NULL;
ProcessTicksService *Controller::processTicksService = NULL;

std::recursive_mutex Controller::lock_io_values_mutex;
ControllerIOValues Controller::cached_io_values = {};

void Controller::Start(EventGroupHandle_t gpio_events) {
    Controller::gpio_events = gpio_events;

    ESP_LOGI(TAG_Controller, "start");

    processTicksService = new ProcessTicksService();
    ladder = new Ladder();
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
    Controller::cached_io_values = {};
    delete ladder;
    delete processTicksService;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;

    ESP_LOGI(TAG_Controller, "start process task");

    ladder->Load();

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
    processTicksService->Request(first_iteration_delay);
    bool need_render = true;
    while (Controller::runned) {
        EventBits_t uxBits = xEventGroupWaitBits(
            Controller::gpio_events,
            BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN
                | BUTTON_RIGHT_IO_CLOSE | BUTTON_RIGHT_IO_OPEN | BUTTON_SELECT_IO_CLOSE
                | BUTTON_SELECT_IO_OPEN | INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN | GPIO_TASK_WAKEUP,
            true,
            false,
            processTicksService->Get());

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
        need_render |= ladder->DoAction();
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
    bool val_1bit;
    uint16_t val_10bit;
    uint8_t percent04;
    ControllerIOValues io_values;

    val_10bit = get_analog_value();
    percent04 = val_10bit / 4;
    io_values.AI = percent04;

    val_1bit = get_digital_input_value();
    percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    io_values.DI = percent04;

    val_1bit = get_digital_value(gpio_output::OUTPUT_0);
    percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    io_values.O1 = percent04;

    val_1bit = get_digital_value(gpio_output::OUTPUT_1);
    percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    io_values.O2 = percent04;

    io_values.V1 = Controller::var1;

    io_values.V2 = Controller::var2;

    io_values.V3 = Controller::var3;

    io_values.V4 = Controller::var4;
    {
        std::lock_guard<std::recursive_mutex> lock(Controller::lock_io_values_mutex);
        bool has_changes =
            memcmp(&io_values, &Controller::cached_io_values, sizeof(io_values)) != 0;
        Controller::cached_io_values = io_values;
        return has_changes;
    }
}

ControllerIOValues &Controller::GetIOValues() {
    std::lock_guard<std::recursive_mutex> lock(Controller::lock_io_values_mutex);
    return Controller::cached_io_values;
}

uint8_t Controller::GetAIRelativeValue() {
    uint8_t percent04 = Controller::cached_io_values.AI;
    ESP_LOGD(TAG_Controller, "adc percent04:%u", percent04);

    const int read_adc_max_period_ms = 1000;
    processTicksService->Request(read_adc_max_period_ms);

    return percent04;
}

uint8_t Controller::GetDIRelativeValue() {
    return Controller::cached_io_values.DI;
}

uint8_t Controller::GetO1RelativeValue() {
    return Controller::cached_io_values.O1;
}
uint8_t Controller::GetO2RelativeValue() {
    return Controller::cached_io_values.O2;
}
uint8_t Controller::GetV1RelativeValue() {
    return Controller::cached_io_values.V1;
}
uint8_t Controller::GetV2RelativeValue() {
    return Controller::cached_io_values.V2;
}
uint8_t Controller::GetV3RelativeValue() {
    return Controller::cached_io_values.V3;
}
uint8_t Controller::GetV4RelativeValue() {
    return Controller::cached_io_values.V4;
}

void Controller::SetO1RelativeValue(uint8_t value) {
    set_digital_value(gpio_output::OUTPUT_0, value != LogicElement::MinValue);
}
void Controller::SetO2RelativeValue(uint8_t value) {
    set_digital_value(gpio_output::OUTPUT_1, value != LogicElement::MinValue);
}
void Controller::SetV1RelativeValue(uint8_t value) {
    Controller::var1 = value;
}
void Controller::SetV2RelativeValue(uint8_t value) {
    Controller::var2 = value;
}
void Controller::SetV3RelativeValue(uint8_t value) {
    Controller::var3 = value;
}
void Controller::SetV4RelativeValue(uint8_t value) {
    Controller::var4 = value;
}

void Controller::RequestWakeupMs(uint32_t delay_ms) {
    processTicksService->Request(delay_ms);
    xEventGroupSetBits(Controller::gpio_events, GPIO_TASK_WAKEUP);
}