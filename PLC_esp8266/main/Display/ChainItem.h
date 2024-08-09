#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class ChainItem : public DisplayItemBase {
  protected:
    Point incoming_point;

  public:
    ChainItem(const Point &incoming_point);
    virtual ~ChainItem();

    virtual Point OutcomingPoint() = 0;
};
