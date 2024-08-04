#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class TimerBase : public InputBase {
  private:
  protected:
    uint64_t raise_time_us;
    bool text_f5X7 = false;
    char str_time[16];

  public:
    TimerBase(uint64_t time_us, InputBase &prior_item);
    ~TimerBase();

    void Render(uint8_t *fb) override final;
};
