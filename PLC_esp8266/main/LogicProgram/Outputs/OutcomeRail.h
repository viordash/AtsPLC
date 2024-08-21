#pragma once

#include "LogicProgram/LogicElement.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class OutcomeRail : public LogicElement {
  private:
    uint8_t network_number;
    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;

  public:
    explicit OutcomeRail(const CommonOutput *incoming_item, uint8_t network_number);
    ~OutcomeRail();

    bool Render(uint8_t *fb, LogicItemState prev_elem_state) override final;
};
