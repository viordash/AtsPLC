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

    virtual void Render(uint8_t *fb) = 0;
};
