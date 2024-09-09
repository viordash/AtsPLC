

#include "LogicProgram/Controller.h"

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/StatusBar.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

#define PROCESS_TASK_STOPPED BIT1
#define RENDER_TASK_STOPPED BIT2
#define DO_RENDERING BIT3

bool Controller::runned = NULL;
EventGroupHandle_t Controller::gpio_events = NULL;
EventGroupHandle_t Controller::events = NULL;

uint8_t Controller::Var1 = LogicElement::MinValue;
uint8_t Controller::Var2 = LogicElement::MinValue;
uint8_t Controller::Var3 = LogicElement::MinValue;
uint8_t Controller::Var4 = LogicElement::MinValue;

Ladder *Controller::ladder = NULL;

void Controller::Start(EventGroupHandle_t gpio_events) {
    Controller::gpio_events = gpio_events;
    Controller::events = xEventGroupCreate();

    ESP_LOGI(TAG_Controller, "start");
    ladder = new Ladder();
    Controller::runned = true;
    ESP_ERROR_CHECK(xTaskCreate(ProcessTask, "ctrl_actions_task", 2048, NULL, 3, NULL) != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void Controller::Stop() {
    Controller::runned = false;

    ESP_LOGI(TAG_Controller, "stop");
    const int tasks_stopping_timeout = 500;
    xEventGroupWaitBits(Controller::events,
                        PROCESS_TASK_STOPPED | RENDER_TASK_STOPPED,
                        true,
                        true,
                        tasks_stopping_timeout / portTICK_PERIOD_MS);

    vEventGroupDelete(Controller::events);
    delete ladder;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;
    ESP_LOGI(TAG_Controller, "start process task");

    ladder->Load();

    ESP_ERROR_CHECK(xTaskCreate(RenderTask, "ctrl_render_task", 2048, NULL, tskIDLE_PRIORITY, NULL)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);

    bool need_render = true;
    while (Controller::runned) {
        const int read_adc_max_period_ms = 100;
        EventBits_t uxBits = xEventGroupWaitBits(Controller::gpio_events,
                                                 INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN,
                                                 true,
                                                 false,
                                                 read_adc_max_period_ms / portTICK_PERIOD_MS);

        need_render |= (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));
        need_render |= ladder->DoAction();

        if (need_render) {
            xEventGroupSetBits(Controller::events, DO_RENDERING);
            need_render = false;
        }
    }

    xEventGroupSetBits(Controller::events, PROCESS_TASK_STOPPED);
    ESP_LOGI(TAG_Controller, "stop process task");
    vTaskDelete(NULL);
}

void Controller::RenderTask(void *parm) {
    (void)parm;
    ESP_LOGI(TAG_Controller, "start render task");

    StatusBar statusBar(0);

    while (Controller::runned) {
        xEventGroupWaitBits(Controller::events, DO_RENDERING, true, false, portMAX_DELAY);

        int64_t now_time = esp_timer_get_time();
        uint8_t *fb = begin_render();
        statusBar.Render(fb);
        ladder->Render(fb);
        end_render(fb);

        ESP_LOGI(TAG_Controller, "r (%d ms)", (int)((esp_timer_get_time() - now_time) / 1000));
    }

    xEventGroupSetBits(Controller::events, RENDER_TASK_STOPPED);
    ESP_LOGI(TAG_Controller, "stop render task");
    vTaskDelete(NULL);
}

uint8_t Controller::GetAIRelativeValue() {
    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    ESP_LOGD(TAG_Controller, "adc, val_10bit:%u, percent04:%u", val_10bit, percent04);
    return percent04;
}

uint8_t Controller::GetDIRelativeValue() {
    bool val_1bit = get_digital_input_value();
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    return percent04;
}

uint8_t Controller::GetO1RelativeValue() {
    uint8_t percent04 =
        get_digital_value(gpio_output::OUTPUT_0) ? LogicElement::MaxValue : LogicElement::MinValue;
    return percent04;
}
uint8_t Controller::GetO2RelativeValue() {
    uint8_t percent04 =
        get_digital_value(gpio_output::OUTPUT_1) ? LogicElement::MaxValue : LogicElement::MinValue;
    return percent04;
}
uint8_t Controller::GetV1RelativeValue() {
    return Controller::Var1;
}
uint8_t Controller::GetV2RelativeValue() {
    return Controller::Var2;
}
uint8_t Controller::GetV3RelativeValue() {
    return Controller::Var3;
}
uint8_t Controller::GetV4RelativeValue() {
    return Controller::Var4;
}

void Controller::SetO1RelativeValue(uint8_t value) {
    set_digital_value(gpio_output::OUTPUT_0, value != LogicElement::MinValue);
}
void Controller::SetO2RelativeValue(uint8_t value) {
    set_digital_value(gpio_output::OUTPUT_1, value != LogicElement::MinValue);
}
void Controller::SetV1RelativeValue(uint8_t value) {
    Controller::Var1 = value;
}
void Controller::SetV2RelativeValue(uint8_t value) {
    Controller::Var2 = value;
}
void Controller::SetV3RelativeValue(uint8_t value) {
    Controller::Var3 = value;
}
void Controller::SetV4RelativeValue(uint8_t value) {
    Controller::Var4 = value;
}