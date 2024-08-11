#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class TimerBase : public CommonInput {
  private:
  protected:
    uint64_t delay_time_us;
    uint64_t raise_time_us;
    char str_time[16];
    int str_size;

    uint64_t GetLeftTime();
    uint8_t GetProgress();

  public:
    explicit TimerBase(InputBase &prev_item);
    ~TimerBase();

    bool Render(uint8_t *fb) override;
};
