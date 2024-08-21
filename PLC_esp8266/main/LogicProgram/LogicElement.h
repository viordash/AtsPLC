#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class IncomeRail;

class LogicElement {
  protected:
    LogicItemState state;

    friend IncomeRail;

  public:
    static const uint8_t MinValue = 0;
    static const uint8_t MaxValue = 250;

    const Controller *controller;
    LogicElement(const Controller *controller);
    virtual ~LogicElement();

    virtual bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) = 0;
    virtual bool Render(uint8_t *fb, LogicItemState prev_elem_state) = 0;

    LogicItemState GetState();
};
