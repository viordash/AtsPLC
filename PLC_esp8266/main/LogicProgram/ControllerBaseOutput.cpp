#include "LogicProgram/ControllerBaseOutput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseOutput::ControllerBaseOutput() {
    Init();
}

ControllerBaseOutput::~ControllerBaseOutput() {
}

void ControllerBaseOutput::Init() {
    required_writing = false;
    out_value = LogicElement::MinValue;
}

void ControllerBaseOutput::WriteValue(uint8_t new_value) {
    required_writing = true;
    out_value = new_value;
}