#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/ControllerBaseOutput.h"
#include "WiFi/WiFiService.h"
#include <stdint.h>
#include <unistd.h>

class ControllerVariable : public ControllerBaseInput, public ControllerBaseOutput {
  protected:
    uint8_t out_value;
    WiFiService *wifi_service;
    const char *ssid;

  public:
    explicit ControllerVariable();
    bool SampleValue() override;
    void WriteValue(uint8_t new_value) override;
    void BindToWiFi(WiFiService *wifi_service, const char *ssid);
    void Unbind();
    bool BindedToWiFi();
};
