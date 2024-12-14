#include "LogicProgram/ControllerInput.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerInput::~ControllerInput() {
}

uint8_t ControllerInput::GetValue() {
    required = true;
    return value;
}
