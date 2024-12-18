#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicElement.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerVariable::ControllerVariable() {
    Unbind();
}

bool ControllerVariable::SampleValue() {
    if (!required) {
        return false;
    }
    required = false;

    return UpdateValue(out_value);
}

void ControllerVariable::SetValue(uint8_t new_value) {
    required = true;
    out_value = new_value;
}

void ControllerVariable::BindToWiFi(const char *ssid) {
    wifi = true;
    this->ssid = ssid;
}

void ControllerVariable::Unbind() {
    wifi = false;
}

bool ControllerVariable::BindedToWiFi() {
    return wifi;
}