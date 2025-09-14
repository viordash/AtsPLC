#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class ScrollBar {
  protected:
  public:
    static bool Render(FrameBuffer *fb, size_t count, size_t viewport_count, size_t view_topindex);
};
