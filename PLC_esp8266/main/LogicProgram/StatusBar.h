#pragma once

#include "Display/Common.h"
#include "Display/MapIOIndicator.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar {
  protected:
    uint8_t y;
    MapIOIndicator indicators[MapIO::V4 + 1];

    void RenderIndicator(FrameBuffer *fb, Point *point, const MapIO io_adr, uint8_t value);

  public:
    explicit StatusBar(uint8_t y);
    StatusBar(const StatusBar &) = delete;
    virtual ~StatusBar();

    StatusBar &operator=(const StatusBar &) = delete;

    void Render(FrameBuffer *fb);
};
