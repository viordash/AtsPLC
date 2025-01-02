#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInput::ControllerBaseInput() {
    Init();
}

ControllerBaseInput::~ControllerBaseInput() {
}

void ControllerBaseInput::Init() {
    required_reading = true;
    in_value = LogicElement::MinValue;
}

uint8_t ControllerBaseInput::ReadValue() {
    required_reading = true;
    return in_value;
}

uint8_t ControllerBaseInput::PeekValue() {
    std::lock_guard<std::mutex> lock(lock_value);
    return in_value;
}

bool ControllerBaseInput::UpdateValue(uint8_t new_value) {
    std::lock_guard<std::mutex> lock(lock_value);
    bool has_changes = in_value != new_value;
    in_value = new_value;
    return has_changes;
}