#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/LogicItemBase.h"
#include <stdint.h>
#include <unistd.h>

class StatusBar : public LogicItemBase, public DisplayItemBase {
  protected:
    uint8_t y;

  public:
    explicit StatusBar(const Controller &controller, uint8_t y);
    virtual ~StatusBar();

    void Render(uint8_t *fb) override final;
    bool DoAction() override final;
};
