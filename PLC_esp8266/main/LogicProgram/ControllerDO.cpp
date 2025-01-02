#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerDO::ControllerDO(gpio_output gpio) : ControllerBaseInput(), ControllerBaseOutput() {
    this->gpio = gpio;
}

void ControllerDO::Init() {
    ControllerBaseInput::Init();
    ControllerBaseOutput::Init();
}

bool ControllerDO::SampleValue() {
    if (!required_reading) {
        return false;
    }
    required_reading = false;

    bool val_1bit = get_digital_value(gpio);
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;
    return UpdateValue(percent04);
}

void ControllerDO::CommitChanges() {
    if (!required_writing) {
        return;
    }
    required_writing = false;
    set_digital_value(gpio, out_value != LogicElement::MinValue);
}
