#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class CommonTimer : public InputBase {
  private:
  protected:
    uint64_t delay_time_us;
    uint64_t raise_time_us;
    char str_time[16];
    int str_size;

    uint64_t GetLeftTime();
    uint8_t GetProgress();

    virtual const Bitmap *GetCurrentBitmap() = 0;

    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 2;
    LogicItemState state = LogicItemState::lisPassive;

  public:
    explicit CommonTimer(InputBase &prev_item);
    ~CommonTimer();

    bool Render(uint8_t *fb) override;
    Point OutcomingPoint() override final;
};
