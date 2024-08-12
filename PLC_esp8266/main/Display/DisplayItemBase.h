#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include <stdint.h>
#include <unistd.h>

class DisplayItemBase {
  protected:
    bool require_render;

  public:
    explicit DisplayItemBase();
    virtual ~DisplayItemBase();

    virtual bool Render(uint8_t *fb) = 0;
};
