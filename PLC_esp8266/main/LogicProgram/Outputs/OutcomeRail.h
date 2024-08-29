#pragma once

#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class OutcomeRail {
  private:
    uint8_t network_number;

  public:
    explicit OutcomeRail(uint8_t network_number);
    ~OutcomeRail();

    bool Render(uint8_t *fb);
};
