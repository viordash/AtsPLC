#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/ControllerBaseOutput.h"
#include "WiFi/WiFiService.h"
#include <stdint.h>
#include <unistd.h>

class ControllerVariable : public ControllerBaseInput, public ControllerBaseOutput {
  protected:
    WiFiService *wifi_service;
    const char *ssid;

  public:
    explicit ControllerVariable();
    void Init() override;
    void FetchValue() override;
    void CommitChanges() override;
    void BindToWiFi(WiFiService *wifi_service, const char *ssid);
    void Unbind();
    bool BindedToWiFi();

    void CancelReadingProcess() override;
};
