#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
}
#endif

#include "LogicProgram/Controller.h"
#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ControllerAI::SampleValue() {
    if (!required) {
        return false;
    }
    if (!Controller::RequestWakeupMs((void *)&Controller::AI, read_adc_max_period_ms)) {
        return false;
    }
    required = false;

    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    return UpdateValue(percent04);
}
