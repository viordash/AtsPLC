#pragma once

#include "LogicProgram/Inputs/TimerBase.h"
#include <stdint.h>
#include <unistd.h>

class TimerMSecs : public TimerBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    TimerMSecs(uint32_t delay_time_ms, InputBase &prior_item);
    ~TimerMSecs();

    bool DoAction() override final;
    void Render(uint8_t *fb) override final;
};
