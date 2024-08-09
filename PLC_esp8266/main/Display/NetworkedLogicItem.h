#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class NetworkedLogicItem : public DisplayItemBase {
  protected:
    Point incoming_point;

  public:
    NetworkedLogicItem(const Point &incoming_point);
    virtual ~NetworkedLogicItem();

    virtual Point OutcomingPoint() = 0;
};
