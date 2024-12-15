#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerAI : public ControllerBaseInput {
  protected:
  public:
    const int read_adc_max_period_ms = 1000;
    bool SampleValue() override;
};
