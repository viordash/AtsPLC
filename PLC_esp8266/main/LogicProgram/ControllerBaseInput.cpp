#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerBaseInput::ControllerBaseInput() {
    required_reading.store(true, std::memory_order_release);
    value.store(LogicElement::MinValue, std::memory_order_release);
}

ControllerBaseInput::~ControllerBaseInput() {
}

void ControllerBaseInput::CancelReadingProcess() {
}

void ControllerBaseInput::Init() {
    required_reading.store(true, std::memory_order_release);
    value.store(LogicElement::MinValue, std::memory_order_release);
}

uint8_t ControllerBaseInput::ReadValue() {
    required_reading.store(true, std::memory_order_release);
    return value.load(std::memory_order_acquire);
}

uint8_t ControllerBaseInput::PeekValue() {
    return value.load(std::memory_order_acquire);
}

void ControllerBaseInput::UpdateValue(uint8_t new_value) {
    value.store(new_value, std::memory_order_release);
}