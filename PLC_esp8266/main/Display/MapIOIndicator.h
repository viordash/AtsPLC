#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class MapIOIndicator {
  protected:
    const char *name;

    static const uint8_t text_width = 6;
    static const uint8_t text_height = 10;
    static const uint8_t name_size = 2;
    static const uint8_t margin = 1;

  public:
    explicit MapIOIndicator(const MapIO io_adr);
    virtual ~MapIOIndicator();

    void Render(FrameBuffer *fb, Point *start_point, uint8_t progress);
    static uint8_t GetHeight();
};
