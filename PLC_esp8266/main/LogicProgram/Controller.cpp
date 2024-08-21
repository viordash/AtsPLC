

#include "LogicProgram/Controller.h"

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/StatusBar.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

uint8_t Controller::Var1 = StatefulElement::MinValue;
uint8_t Controller::Var2 = StatefulElement::MinValue;
uint8_t Controller::Var3 = StatefulElement::MinValue;
uint8_t Controller::Var4 = StatefulElement::MinValue;

Controller::Controller(EventGroupHandle_t gpio_events) {
    runned = false;
    this->gpio_events = gpio_events;
}

Controller::~Controller() {
    Stop();
}

void Controller::Start() {
    runned = true;
    ESP_ERROR_CHECK(xTaskCreate(ProcessTask, "controller_task", 4096, this, 3, NULL) != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void Controller::Stop() {
    runned = false;
}

void Controller::ProcessTask(void *parm) {
    ESP_LOGI(TAG_Controller, "start ++++++");
    Controller *controller = (Controller *)parm;
    StatusBar statusBar(controller, 0);

    IncomeRail incomeRail0(controller, 0, LogicItemState::lisActive);

    incomeRail0.Append(new InputNO(MapIO::DI, &incomeRail0));

    InputNO input00(MapIO::DI, &incomeRail0);
    InputNC input01(MapIO::V1, &input00);
    TimerMSecs timer00(500, &input01);
    SetOutput output00(MapIO::V1, &timer00);
    OutcomeRail outcomeRail0(&output00, 0);

    IncomeRail incomeRail1(controller, 1, LogicItemState::lisActive);
    InputNO input10(MapIO::DI, &incomeRail1);
    InputNO input11(MapIO::V1, &input10);
    TimerMSecs timer10(500, &input11);
    ResetOutput output10(MapIO::V1, &timer10);
    OutcomeRail outcomeRail1(&output10, 1);

    bool need_render = true;
    while (controller->runned) {
        const int read_adc_max_period_ms = 100;
        EventBits_t uxBits = xEventGroupWaitBits(controller->gpio_events,
                                                 INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN,
                                                 true,
                                                 false,
                                                 read_adc_max_period_ms / portTICK_PERIOD_MS);

        need_render |= (uxBits & (INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN));

        need_render |= incomeRail0.DoAction();
        need_render |= incomeRail1.DoAction();

        // need_render |= timer00.ProgressHasChanges();
        // need_render |= timer10.ProgressHasChanges();

        if (need_render) {
            ESP_LOGI(TAG_Controller, ".");

            uint8_t *fb = begin_render();

            statusBar.Render(fb);

            incomeRail0.Render(fb, LogicItemState::lisActive);
            outcomeRail0.Render(fb, LogicItemState::lisActive);

            incomeRail1.Render(fb, LogicItemState::lisActive);
            outcomeRail1.Render(fb, LogicItemState::lisActive);

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
    uint8_t percent04 = val_1bit ? StatefulElement::MaxValue : StatefulElement::MinValue;
    return percent04;
}

uint8_t Controller::GetO1RelativeValue() {
    uint8_t percent04 = get_digital_value(gpio_output::OUTPUT_0) ? StatefulElement::MaxValue
                                                                 : StatefulElement::MinValue;
    return percent04;
}
uint8_t Controller::GetO2RelativeValue() {
    uint8_t percent04 = get_digital_value(gpio_output::OUTPUT_1) ? StatefulElement::MaxValue
                                                                 : StatefulElement::MinValue;
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
    set_digital_value(gpio_output::OUTPUT_0, value != StatefulElement::MinValue);
}
void Controller::SetO2RelativeValue(uint8_t value) {
    set_digital_value(gpio_output::OUTPUT_1, value != StatefulElement::MinValue);
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