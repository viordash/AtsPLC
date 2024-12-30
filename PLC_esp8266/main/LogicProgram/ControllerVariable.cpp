#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerVariable = "ControllerVariable";

ControllerVariable::ControllerVariable() {
    Unbind();
}

bool ControllerVariable::SampleValue() {
    if (!required) {
        return false;
    }
    required = false;

    if (BindedToWiFi()) {
        out_value = wifi_service->Scan(ssid) ? LogicElement::MaxValue : LogicElement::MinValue;
    }
    return UpdateValue(out_value);
}

void ControllerVariable::WriteValue(uint8_t new_value) {
    required = true;
    out_value = new_value;
    if (out_value && BindedToWiFi()) {
        wifi_service->Generate(ssid);
    }
}

void ControllerVariable::BindToWiFi(WiFiService *wifi_service, const char *ssid) {
    this->wifi_service = wifi_service;
    this->ssid = ssid;
}

void ControllerVariable::Unbind() {
    this->wifi_service = NULL;
}

bool ControllerVariable::BindedToWiFi() {
    return this->wifi_service != NULL;
}

void ControllerVariable::CancelReadingValue() {
    ESP_LOGI(TAG_ControllerVariable,
             "CancelReadingValue, wifi:%u, required:%u",
             BindedToWiFi(),
             required);
    if (BindedToWiFi()) {
        wifi_service->CancelScan(ssid);
    }
}