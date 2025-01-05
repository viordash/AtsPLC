#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerVariable = "ControllerVariable";

ControllerVariable::ControllerVariable() : ControllerBaseInput(), ControllerBaseOutput() {
    Unbind();
}

void ControllerVariable::Init() {
    ControllerBaseInput::Init();
    ControllerBaseOutput::Init();
}

void ControllerVariable::FetchValue() {
    if (!required_reading) {
        return;
    }
    required_reading = false;

    uint8_t val;
    if (BindedToWiFi()) {
        val = wifi_service->Scan(ssid) ? LogicElement::MaxValue : LogicElement::MinValue;
    } else {
        val = out_value;
    }
    UpdateValue(val);
}

void ControllerVariable::CommitChanges() {
    if (!required_writing) {
        return;
    }
    required_writing = false;
    UpdateValue(out_value);
    if (BindedToWiFi()) {
        if (out_value != LogicElement::MinValue) {
            wifi_service->Generate(ssid);
        } else {
            wifi_service->CancelGenerate(ssid);
        }
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

void ControllerVariable::CancelReadingProcess() {
    ESP_LOGI(TAG_ControllerVariable,
             "CancelReadingProcess, wifi:%u, required:%u",
             BindedToWiFi(),
             required_reading);
    if (BindedToWiFi()) {
        wifi_service->CancelScan(ssid);
    }
    value = LogicElement::MinValue;
    out_value = LogicElement::MinValue;
}