#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerMSecs : public CommonTimer {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    TimerMSecs(uint32_t delay_time_ms, InputBase &prev_item);
    ~TimerMSecs();

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
};
