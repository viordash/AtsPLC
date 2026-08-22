#include "LogicProgram/ControllerBaseInputOutput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInputOutput::ControllerBaseInputOutput() : ControllerBaseInput() {
    required_writing.store(false, std::memory_order_release);
    out_value = LogicElement::MinValue;
}

ControllerBaseInputOutput::~ControllerBaseInputOutput() {
}

void ControllerBaseInputOutput::Init() {
    ControllerBaseInput::Init();
    required_writing.store(false, std::memory_order_release);
    out_value = LogicElement::MinValue;
}

void ControllerBaseInputOutput::WriteValue(uint8_t new_value) {
    required_writing.store(true, std::memory_order_release);
    out_value = new_value;
}