#pragma once

#include "LogicProgram/ControllerInput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDI : public ControllerInput {
  protected:
  public:
    void SampleValue() override;
};
