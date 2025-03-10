#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerAI::ControllerAI() : ControllerBaseInput() {
}

void ControllerAI::FetchValue() {
    if (!required_reading) {
        return;
    }
    if (!Controller::RequestWakeupMs((void *)&Controller::AI,
                                     read_adc_max_period_ms,
                                     ProcessWakeupRequestPriority::pwrp_Normal)) {
        return;
    }
    required_reading = false;

    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    UpdateValue(percent04);
}
