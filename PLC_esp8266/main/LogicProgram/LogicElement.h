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
    LogicElement *nextElement;

    friend IncomeRail;

  public:
    const Controller *controller;
    LogicElement(const Controller *controller);
    virtual ~LogicElement();

    virtual bool DoAction() = 0;
    LogicItemState GetState();

    void Bind(LogicElement *element);
};
