#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class DisplayChainItem {
  protected:
    Point incoming_point;

  public:
    DisplayChainItem(const Point &incoming_point);

    virtual ~DisplayChainItem();
    virtual Point OutcomingPoint() = 0;
};
