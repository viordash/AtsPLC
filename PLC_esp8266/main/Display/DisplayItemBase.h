#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  protected:
    Point location;
    const static Bitmap bitmap;
    static void draw(uint8_t *fb, int8_t x, int8_t y, const Bitmap &bitmap);

  public:
    explicit DisplayItemBase(const Point &location);
    virtual ~DisplayItemBase();

    virtual const Bitmap &GetBitmap() = 0;
    const Point &GetLocation();
    virtual void Render(const uint8_t *fb) = 0;
};
