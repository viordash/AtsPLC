#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class MapIOIndicator {
  protected:
    Point incoming_point;
    const char *name;
    uint8_t progress;
    uint8_t separator_width;

    static const uint8_t text_width = 6;
    static const uint8_t text_height = 10;
    static const uint8_t name_size = 2;
    static const uint8_t margin = 1;

  public:
    explicit MapIOIndicator(const Point &incoming_point,
                            const char *name,
                            uint8_t progress,
                            uint8_t separator_width);
    virtual ~MapIOIndicator();

    bool Render(uint8_t *fb);
    static uint8_t GetWidth();
    static uint8_t GetHeight();
};
