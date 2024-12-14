#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ControllerDI::SampleValue() {
    if (!required) {
        return;
    }

    bool val_1bit = get_digital_input_value();
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    value = percent04;
    required = false;
}
