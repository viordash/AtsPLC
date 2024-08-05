#pragma once

#include "Display/Common.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class OutputBase;
class LogicItemBase {
  protected:
    LogicItemState state = LogicItemState::lisPassive;
    
    friend OutputBase;

  public:

    LogicItemBase();
    virtual ~LogicItemBase();

    virtual bool DoAction() = 0;
};
