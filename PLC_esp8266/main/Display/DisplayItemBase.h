#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  private:
    Point location;
    Size size;

  public:
    DisplayItemBase(const Point &location, const Size &size);
    virtual ~DisplayItemBase();

    virtual const uint8_t *GetBitmap() = 0;
    const Point &GetLocation();
    const Size &GetSize();
};
