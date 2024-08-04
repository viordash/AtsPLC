#pragma once

#include "LogicProgram/Inputs/TimerBase.h"
#include <stdint.h>
#include <unistd.h>

class TimerSecs : public TimerBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    TimerSecs(uint16_t delay_time_s, InputBase &prior_item);
    ~TimerSecs();

    bool DoAction() override final;
};
