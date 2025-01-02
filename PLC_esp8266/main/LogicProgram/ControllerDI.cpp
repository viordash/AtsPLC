#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerDI::ControllerDI() : ControllerBaseInput() {
}

bool ControllerDI::FetchValue() {
    if (!required_reading) {
        return false;
    }
    required_reading = false;

    bool val_1bit = get_digital_input_value();
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    return UpdateValue(percent04);
}
