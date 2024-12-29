#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInput::~ControllerBaseInput() {
}

void ControllerBaseInput::Init() {
    required = true;
    value = LogicElement::MinValue;
}

uint8_t ControllerBaseInput::ReadValue() {
    required = true;
    return value;
}

uint8_t ControllerBaseInput::PeekValue() {
    std::lock_guard<std::mutex> lock(lock_value);
    return value;
}

bool ControllerBaseInput::UpdateValue(uint8_t new_value) {
    std::lock_guard<std::mutex> lock(lock_value);
    bool has_changes = value != new_value;
    value = new_value;
    return has_changes;
}