#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  protected:
    Point location;
    const static Bitmap bitmap;

  public:
    DisplayItemBase(const Point &location);
    virtual ~DisplayItemBase();

    virtual const Bitmap &GetBitmap() = 0;
    const Point &GetLocation();
};
