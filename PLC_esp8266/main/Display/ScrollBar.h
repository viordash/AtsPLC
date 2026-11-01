#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class ScrollBar {
  protected:
  public:
    static void Render(FrameBuffer *fb,
                       uint16_t left,
                       uint16_t top,
                       uint16_t scroll_height,
                       size_t count,
                       size_t viewport_count,
                       size_t view_topindex);
};
