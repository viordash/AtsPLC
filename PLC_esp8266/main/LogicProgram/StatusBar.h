#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar {
  protected:
    uint8_t y;

  public:
    explicit StatusBar(const Controller *controller, uint8_t y);
    virtual ~StatusBar();

    bool Render(uint8_t *fb);
};
