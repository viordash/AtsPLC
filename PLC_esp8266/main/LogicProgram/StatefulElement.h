#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class IncomeRail;

class StatefulElement {
  protected:
    LogicItemState state;
    StatefulElement *nextElement;

    friend IncomeRail;

  public:
    static const uint8_t MinValue = 0;
    static const uint8_t MaxValue = 250;

    const Controller *controller;
    StatefulElement(const Controller *controller);
    virtual ~StatefulElement();

    virtual bool DoAction(bool prev_changed) = 0;
    LogicItemState GetState();

    void Bind(StatefulElement *element);
};
