#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class IncomeRail : public InputBase {
  private:
    uint8_t network_number;

  public:
    explicit IncomeRail(const Controller *controller, uint8_t network_number);
    virtual ~IncomeRail();

    Point OutcomingPoint() override final;

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
};
