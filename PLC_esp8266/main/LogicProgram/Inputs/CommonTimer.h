#pragma once

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
    LogicItemState incoming_item_prev_state;

    uint64_t GetLeftTime();
    uint8_t GetProgress();

    virtual const Bitmap *GetCurrentBitmap() = 0;

    const uint8_t LeftPadding = 14;
    const uint8_t RightPadding = 2;

    bool IncomingItemStateHasChanged();

  public:
    explicit CommonTimer(InputBase *incoming_item);
    ~CommonTimer();

    bool Render(uint8_t *fb) override;
    Point OutcomingPoint() override final;
};
