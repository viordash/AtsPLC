#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class OutputBase;
class LogicElement {
  protected:
    Controller controller;
    LogicItemState state = LogicItemState::lisPassive;

    friend OutputBase;

  public:
    LogicElement(const Controller &controller);
    virtual ~LogicElement();

    virtual bool DoAction() = 0;
};
