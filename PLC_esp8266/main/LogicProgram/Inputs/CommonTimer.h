#pragma once

#include "Display/display.h"
#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class CommonTimer : public InputBase {
  private:
  protected:
    uint64_t delay_time_us;
    uint64_t start_time_us;
    char str_time[16];
    int str_size;
    InputBase *incoming_item;

    uint64_t GetLeftTime();
    uint8_t GetProgress();

    virtual const Bitmap *GetCurrentBitmap() = 0;

    const uint8_t LeftPadding = 4;
    const uint8_t RightPadding = 0;

  public:
    explicit CommonTimer(InputBase *incoming_item);
    ~CommonTimer();

    bool DoAction(bool prev_changed) override;
    bool Render(uint8_t *fb, LogicItemState prev_state) override;
    Point OutcomingPoint() override final;
};
