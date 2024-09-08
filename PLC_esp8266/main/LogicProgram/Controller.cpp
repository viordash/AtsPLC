

#include "LogicProgram/Controller.h"

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/StatusBar.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

bool Controller::runned = false;
EventGroupHandle_t Controller::gpio_events = NULL;

uint8_t Controller::Var1 = LogicElement::MinValue;
uint8_t Controller::Var2 = LogicElement::MinValue;
uint8_t Controller::Var3 = LogicElement::MinValue;
uint8_t Controller::Var4 = LogicElement::MinValue;

void Controller::Start(EventGroupHandle_t gpio_events) {
    Controller::gpio_events = gpio_events;
    Controller::runned = true;
    ESP_ERROR_CHECK(xTaskCreate(ProcessTask, "controller_task", 4096, NULL, 3, NULL) != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void Controller::Stop() {
    Controller::runned = false;
}

void Controller::ProcessTask(void *parm) {
    (void)parm;
    ESP_LOGI(TAG_Controller, "start ++++++");

    StatusBar statusBar(0);
    Ladder ladder;
    ladder.Load();

    bool need_render = true;
    while (Controller::runned) {
        const int read_adc_max_period_ms = 100;
        EventBits_t uxBits = xEventGroupWaitBits(Controller::gpio_events,
                                                 INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN,
                                                 true,
                                                 false,
                                                 read_adc_max_period_ms / portTICK_PERIOD_MS);

        need_render |= (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));

        for (auto network : ladder) {
            need_render |= network->DoAction();
        }

        if (need_render) {
            ESP_LOGI(TAG_Controller, ".");

            uint8_t *fb = begin_render();

            statusBar.Render(fb);

            for (auto network : ladder) {
                need_render |= network->Render(fb);
            }

            end_render(fb);
            need_render = false;
        }
    }

    ESP_LOGI(TAG_Controller, "stop -------");
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