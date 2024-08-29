#pragma once

#include "Display/Common.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputBase : public LogicElement{
  protected:
    InputBase(const Controller *controller);

  public:
    virtual ~InputBase();
};
