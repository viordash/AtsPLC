#pragma once

#include "LogicProgram/Common.h"
#include <stdint.h>
#include <unistd.h>

class ControllerInput {
  protected:
    uint8_t value;
    bool required;

  public:
    virtual ~ControllerInput();

    virtual void SampleValue() = 0;
    uint8_t GetValue();
};
