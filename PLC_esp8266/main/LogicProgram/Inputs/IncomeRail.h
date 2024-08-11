#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class IncomeRail : public InputBase {
  private:
    uint8_t network_number;
    const Bitmap *GetCurrentBitmap() override final;

  public:
    explicit IncomeRail(const Controller &controller, uint8_t network_number);
    ~IncomeRail();

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
    Point OutcomingPoint() override final;
};
