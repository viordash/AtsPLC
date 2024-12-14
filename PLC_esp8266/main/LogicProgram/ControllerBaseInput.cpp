#include "LogicProgram/ControllerBaseInput.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInput::~ControllerBaseInput() {
}

uint8_t ControllerBaseInput::GetValue() {
    required = true;
    return value;
}

uint8_t ControllerBaseInput::PeekValue() {
    std::lock_guard<std::mutex> lock(lock_value);
    return value;
}

bool ControllerBaseInput::UpdateValue(uint8_t new_value) {
    std::lock_guard<std::mutex> lock(lock_value);
    bool has_changes = value == new_value;
    value = new_value;
    return has_changes;
}