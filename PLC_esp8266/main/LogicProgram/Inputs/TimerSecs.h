#pragma once

#include "LogicProgram/Inputs/CommonTimer.h"
#include <stdint.h>
#include <unistd.h>

class TimerSecs : public CommonTimer {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;
    const static uint64_t force_render_period_us = 1000000;

    uint64_t force_render_time_us;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    TimerSecs(uint32_t delay_time_s, InputBase *incoming_item);
    ~TimerSecs();

    bool Render(uint8_t *fb, LogicItemState prev_state) override final;
    bool ProgressHasChanges();
};
