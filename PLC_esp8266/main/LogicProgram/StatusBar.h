#pragma once

#include "Display/Common.h"
#include "LogicProgram/StatefulElement.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar : public StatefulElement {
  protected:
    uint8_t y;

  public:
    explicit StatusBar(const Controller *controller, uint8_t y);
    virtual ~StatusBar();

    bool Render(uint8_t *fb) override final;
    bool DoAction(bool prev_changed) override final;
};
