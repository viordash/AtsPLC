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

  protected:
    bool ProgressHasChanges(LogicItemState prev_elem_state);

  public:
    TimerSecs(uint32_t delay_time_s, InputBase *incoming_item);
    ~TimerSecs();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override final;
};
