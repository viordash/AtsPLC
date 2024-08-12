

#include "LogicProgram/Controller.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Controller::Controller(/* args */) {
}

Controller::~Controller() {
}

uint8_t Controller::GetAIRelativeValue() {
    uint16_t val_10bit = get_analog_value();
    uint8_t val_half_percent = val_10bit / 5;
    return val_half_percent;
}

uint8_t Controller::GetDIRelativeValue() {
    bool val_1bit = get_digital_input_value();
    uint8_t val_half_percent = val_1bit ? 200 : 0;
    return val_half_percent;
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