#include "LogicProgram/ControllerBaseInputOutput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInputOutput::ControllerBaseInputOutput() : ControllerBaseInput() {
}

ControllerBaseInputOutput::~ControllerBaseInputOutput() {
}

void ControllerBaseInputOutput::Setup() {
    ControllerBaseInput::Setup();
    required_writing = false;
    out_value = LogicElement::MinValue;
}

void ControllerBaseInputOutput::WriteValue(uint8_t new_value) {
    required_writing = true;
    out_value = new_value;
}