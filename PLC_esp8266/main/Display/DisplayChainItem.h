#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/LogicItemState.h"
#include <stdint.h>
#include <unistd.h>

class CommonInput;

class DisplayChainItem : public DisplayItemBase {
  protected:
    Point incoming_point;

    friend CommonInput;

  public:
    DisplayChainItem(const Point &incoming_point);
    DisplayChainItem(DisplayChainItem *incoming_item);

    virtual ~DisplayChainItem();
    virtual Point OutcomingPoint() = 0;
};
