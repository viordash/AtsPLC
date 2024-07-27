#pragma once

#include "Display/Common.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogicItemBase {
  protected:
    LogicItemState state;

  public:
    LogicItemBase();
    virtual ~LogicItemBase();

    virtual bool DoAction() = 0;
};
