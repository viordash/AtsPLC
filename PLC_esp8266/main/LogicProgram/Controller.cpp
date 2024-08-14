

#include "LogicProgram/Controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
uint16_t get_analog_value();
bool get_digital_input_value();
}

Controller::Controller(/* args */) {
}

Controller::~Controller() {
}

uint8_t Controller::GetAIRelativeValue() {
    uint16_t val_10bit = get_analog_value();
    uint8_t percent04 = val_10bit / 4;
    return percent04;
}

uint8_t Controller::GetDIRelativeValue() {
    bool val_1bit = get_digital_input_value();
    uint8_t percent04 = val_1bit ? 250 : 0;
    return percent04;
}

uint8_t Controller::GetO1RelativeValue() {
    return 100;
}
uint8_t Controller::GetO2RelativeValue() {
    return 100;
}
uint8_t Controller::GetV1RelativeValue() {
    return 25;
}
uint8_t Controller::GetV2RelativeValue() {
    return 50;
}
uint8_t Controller::GetV3RelativeValue() {
    return 75;
}
uint8_t Controller::GetV4RelativeValue() {
    return 0;
}