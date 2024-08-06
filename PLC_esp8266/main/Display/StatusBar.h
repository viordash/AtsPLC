#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar : public DisplayItemBase {
  protected:
    uint8_t y;

  public:
    explicit StatusBar(uint8_t y);
    virtual ~StatusBar();

    void Render(uint8_t *fb) override final;
    void SetOrigin() override final;
};
