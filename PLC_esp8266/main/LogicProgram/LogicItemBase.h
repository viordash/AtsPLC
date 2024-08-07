#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class OutputBase;
class LogicItemBase {
  protected:
    Controller controller;
    LogicItemState state = LogicItemState::lisPassive;

    friend OutputBase;

  public:
    LogicItemBase(const Controller &controller);
    virtual ~LogicItemBase();

    virtual bool DoAction() = 0;
};
