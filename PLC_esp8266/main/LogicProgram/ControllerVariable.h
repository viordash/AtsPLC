#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/ControllerBaseOutput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerVariable : public ControllerBaseInput, public ControllerBaseOutput {
  protected:
    uint8_t out_value;
    bool wifi;
    const char *ssid;

  public:
    explicit ControllerVariable();
    bool SampleValue() override;
    void SetValue(uint8_t new_value) override;
    void BindToWiFi(const char *ssid);
    void Unbind();
};
