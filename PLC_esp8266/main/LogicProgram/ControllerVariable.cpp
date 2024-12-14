#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ControllerVariable::SampleValue() {
    if (!required) {
        return false;
    }
    required = false;

    return UpdateValue(out_value);
}

void ControllerVariable::SetValue(uint8_t new_value) {
    out_value = new_value;
}