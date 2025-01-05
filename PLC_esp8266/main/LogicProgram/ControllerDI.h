#pragma once

#include "LogicProgram/ControllerBaseInput.h"
#include <stdint.h>
#include <unistd.h>

class ControllerDI : public ControllerBaseInput {
  protected:
  public:
    ControllerDI();
    void FetchValue() override;
};
