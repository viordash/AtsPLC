#pragma once

#include "Display/Common.h"
#include "Display/DisplayChainItem.h"
#include "LogicProgram/StatefulElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputBase : public StatefulElement, public DisplayChainItem {
  protected:
    InputBase(const Controller *controller, const Point &incoming_point);

  public:
    virtual ~InputBase();
};
