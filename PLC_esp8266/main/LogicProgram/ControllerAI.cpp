#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerAI = "ControllerAI";

extern CurrentSettings::device_settings settings;

ControllerAI::ControllerAI() : ControllerBaseInput() {
}

void ControllerAI::FetchValue() {
    if (!required_reading) {
        return;
    }
    if (!Controller::RequestWakeupMs((void *)&Controller::AI,
                                     settings.adc.scan_period_ms,
                                     ProcessWakeupRequestPriority::pwrp_Idle)) {
        return;
    }
    required_reading = false;

    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    UpdateValue(percent04);
    ESP_LOGD(TAG_ControllerAI, "fetch value");
}
