#pragma once

#include "LogicProgram/Outputs/OutputBase.h"
#include <stdint.h>
#include <unistd.h>

class OutcomeRail : public OutputBase {
  private:
    uint8_t network_number;
    const Bitmap *GetCurrentBitmap() override final;

  public:
    explicit OutcomeRail(const Controller &controller, uint8_t network_number);
    ~OutcomeRail();

    bool DoAction() override final;
    void Render(uint8_t *fb) override final;
    Point OutcomingPoint() override final;
};
