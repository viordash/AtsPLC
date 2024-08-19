#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class IncomeRail;

class LogicElement : public DisplayItemBase {
  protected:
    LogicElement *nextElement;

    friend IncomeRail;

  public:
    static const uint8_t MinValue = 0;
    static const uint8_t MaxValue = 250;

    const Controller *controller;
    LogicElement(const Controller *controller);
    virtual ~LogicElement();

    virtual bool DoAction(bool prev_changed) = 0;

    void Bind(LogicElement *element);
};
