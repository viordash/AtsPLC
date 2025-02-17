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
        bool insecure_scan = ssid != NULL && password == NULL && mac == NULL;
        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;

        if (insecure_scan) {
            val = wifi_service->Scan(ssid);
        } else if (secure_ap) {
            val = wifi_service->AccessPoint(ssid, password, mac);
        } else {
            val = wifi_service->ConnectToStation();
        }
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
        bool wifi_sta_client = ssid == NULL;
        if (wifi_sta_client) {
            return;
        }

        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;
        if (secure_ap) {
            return;
        }

        if (out_value != LogicElement::MinValue) {
            wifi_service->AccessPoint(ssid, password, mac);
        } else {
            wifi_service->CancelAccessPoint(ssid);
        }
    }
}

void ControllerVariable::BindToSecureWiFi(WiFiService *wifi_service,
                                          const char *ssid,
                                          const char *password,
                                          const char *mac) {
    this->wifi_service = wifi_service;
    this->ssid = ssid;
    this->password = password;
    this->mac = mac;
}

void ControllerVariable::BindToInsecureWiFi(WiFiService *wifi_service, const char *ssid) {
    this->wifi_service = wifi_service;
    this->ssid = ssid;
    this->password = NULL;
    this->mac = NULL;
}

void ControllerVariable::BindToStaWiFi(WiFiService *wifi_service) {
    this->wifi_service = wifi_service;
    this->ssid = NULL;
    this->password = NULL;
    this->mac = NULL;
}

void ControllerVariable::Unbind() {
    this->wifi_service = NULL;
}

bool ControllerVariable::BindedToWiFi() {
    return this->wifi_service != NULL;
}

void ControllerVariable::CancelReadingProcess() {
    ESP_LOGD(TAG_ControllerVariable,
             "CancelReadingProcess, wifi:%u, required:%u",
             BindedToWiFi(),
             required_reading);
    if (BindedToWiFi()) {
        bool insecure_scan = ssid != NULL && password == NULL && mac == NULL;
        bool secure_ap = ssid != NULL && password != NULL && mac != NULL;

        if (insecure_scan) {
            wifi_service->CancelScan(ssid);
        } else if (secure_ap) {
            wifi_service->CancelAccessPoint(ssid);
        } else {
            wifi_service->DisconnectFromStation();
        }
        value = LogicElement::MinValue;
        out_value = LogicElement::MinValue;
    }
}