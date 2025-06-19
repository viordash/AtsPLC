#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerAI : public ControllerBaseInput {
  protected:
  public:
    ControllerAI();
    void FetchValue() override;
};
