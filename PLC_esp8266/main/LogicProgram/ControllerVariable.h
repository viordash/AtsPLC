#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/ControllerBaseOutput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerVariable : public ControllerBaseInput, public ControllerBaseOutput {
  protected:
  public:
    bool SampleValue() override;
    void SetValue(uint8_t new_value) override;
};
