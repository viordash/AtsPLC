#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerDO::ControllerDO(gpio_output gpio) {
    this->gpio = gpio;
}

bool ControllerDO::SampleValue() {
    if (!required) {
        return false;
    }
    required = false;

    bool val_1bit = get_digital_value(gpio);
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    return UpdateValue(percent04);
}

void ControllerDO::WriteValue(uint8_t new_value) {
    required = true;
    set_digital_value(gpio, new_value != LogicElement::MinValue);
}