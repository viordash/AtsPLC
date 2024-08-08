#pragma once

#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class OutcomeRail : public DisplayItemBase {
  private:
    uint8_t network_number;

  public:
    explicit OutcomeRail(uint8_t network_number);
    ~OutcomeRail();

    void Render(uint8_t *fb) override final;
};
