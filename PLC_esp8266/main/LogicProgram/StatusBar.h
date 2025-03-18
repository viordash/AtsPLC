#pragma once

#include "Display/Common.h"
#include "Display/MapIOIndicator.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar {
  protected:
    uint8_t y;
    MapIOIndicator *indicator_AI;
    MapIOIndicator *indicator_DI;
    MapIOIndicator *indicator_O1;
    MapIOIndicator *indicator_O2;
    MapIOIndicator *indicator_V1;
    MapIOIndicator *indicator_V2;
    MapIOIndicator *indicator_V3;
    MapIOIndicator *indicator_V4;

  public:
    explicit StatusBar(uint8_t y);
    StatusBar(const StatusBar&) = delete;
    virtual ~StatusBar();

    StatusBar & operator=(const StatusBar&) = delete;

    bool Render(uint8_t *fb);
};
