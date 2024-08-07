#include "LogicProgram/Controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Controller::Controller(/* args */) {
}

Controller::~Controller() {
}

uint8_t Controller::GetAIRelativeValue() {
    return 10;
}
uint8_t Controller::GetDIRelativeValue() {
    return 100;
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