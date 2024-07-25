#pragma once

#include "Display/Location.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogicItemBase {
  private:
  public:
    LogicItemBase();
    virtual ~LogicItemBase();

    virtual bool DoAction() = 0;
};
