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

    uint64_t GetLeftTime();
    uint8_t GetProgress(LogicItemState prev_elem_state);

    virtual const Bitmap *GetCurrentBitmap() = 0;

    const uint8_t LeftPadding = 4;
    const uint8_t RightPadding = 0;

  public:
    explicit CommonTimer(const Controller *controller);
    ~CommonTimer();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override;
    Point OutcomingPoint();
};
