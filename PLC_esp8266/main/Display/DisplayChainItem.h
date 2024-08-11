#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/LogicItemState.h"
#include <stdint.h>
#include <unistd.h>

class DisplayChainItem : public DisplayItemBase {
  protected:
    Point incoming_point;

  public:
    DisplayChainItem(const Point &incoming_point);

    virtual ~DisplayChainItem();
    virtual Point OutcomingPoint() = 0;
};
