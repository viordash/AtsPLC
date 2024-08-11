#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerSecs : public CommonTimer {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    TimerSecs(uint32_t delay_time_s, InputBase *incoming_item);
    ~TimerSecs();

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
};
