#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogicElement {
  protected:
    LogicItemState state;

  public:
    Controller controller;
    LogicElement(const Controller &controller, LogicItemState init_state);
    virtual ~LogicElement();

    virtual bool DoAction() = 0;
    LogicItemState GetState();
};
