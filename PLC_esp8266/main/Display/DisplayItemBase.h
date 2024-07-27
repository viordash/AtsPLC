#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  protected:
    Point incoming_point;

  public:
    explicit DisplayItemBase(const Point &incoming_point);
    virtual ~DisplayItemBase();

    virtual void Render(uint8_t *fb) = 0;
    virtual Point OutcomingPoint() = 0;
};
