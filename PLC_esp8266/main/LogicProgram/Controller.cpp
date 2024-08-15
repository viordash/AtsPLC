

#include "LogicProgram/Controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Display/Common.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/StatusBar.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Controller = "controller";

extern "C" {
uint16_t get_analog_value();
bool get_digital_input_value();
}

Controller::Controller(/* args */) {
    runned = false;
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

    IncomeRail incomeRail0(controller, 0);

    InputNO input0(MapIO::DI, &incomeRail0);
    TimerSecs timerSecs0(5, &input0);
    DirectOutput directOutput0(MapIO::O1, &timerSecs0);
    OutcomeRail outcomeRail0(0);

    IncomeRail incomeRail1(controller, 1);
    ComparatorGE comparator1(15, MapIO::AI, &incomeRail1);
    TimerSecs timerSecs1(10, &comparator1);
    DirectOutput directOutput1(MapIO::O2, &timerSecs1);
    OutcomeRail outcomeRail1(1);

    while (controller->runned) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        bool need_render = false;
        need_render |= incomeRail0.DoAction();
        need_render |= incomeRail1.DoAction();

        need_render |= timerSecs0.ProgressHasChanges();
        need_render |= timerSecs1.ProgressHasChanges();

        if (need_render) {
            ESP_LOGI(TAG_Controller, ".");

            uint8_t *fb = begin_render();

            statusBar.Render(fb);
            incomeRail0.Render(fb);
            input0.Render(fb);
            timerSecs0.Render(fb);
            directOutput0.Render(fb);
            outcomeRail0.Render(fb);

            incomeRail1.Render(fb);
            comparator1.Render(fb);
            timerSecs1.Render(fb);
            directOutput1.Render(fb);
            outcomeRail1.Render(fb);
            end_render(fb);
        }
    }

    ESP_LOGI(TAG_Controller, "stop -------");
    vTaskDelete(NULL);
}

uint8_t Controller::GetAIRelativeValue() {
    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    return percent04;
}

uint8_t Controller::GetDIRelativeValue() {
    bool val_1bit = get_digital_input_value();
    uint8_t percent04 = val_1bit ? 250 : 0;
    return percent04;
}

uint8_t Controller::GetO1RelativeValue() {
    return 100;
}
uint8_t Controller::GetO2RelativeValue() {
    return 100;
}
uint8_t Controller::GetV1RelativeValue() {
    return 25;
}
uint8_t Controller::GetV2RelativeValue() {
    return 50;
}
uint8_t Controller::GetV3RelativeValue() {
    return 75;
}
uint8_t Controller::GetV4RelativeValue() {
    return 0;
}
