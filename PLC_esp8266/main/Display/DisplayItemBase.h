#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  protected:
    Point origin;

  public:
    explicit DisplayItemBase();
    virtual ~DisplayItemBase();

    virtual void Render(uint8_t *fb) = 0;
};
