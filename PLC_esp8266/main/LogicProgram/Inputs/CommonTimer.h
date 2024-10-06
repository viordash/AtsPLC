#pragma once

#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonTimer : public LogicElement {
  private:
  protected:
    uint64_t delay_time_us;
    uint64_t start_time_us;
    char str_time[16];
    int str_size;

    uint64_t GetLeftTime();
    uint8_t GetProgress(LogicItemState prev_elem_state);

    virtual const Bitmap *GetCurrentBitmap(LogicItemState state) = 0;

    const uint8_t LeftPadding = 8;

  public:
    explicit CommonTimer();
    ~CommonTimer();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;

    static CommonTimer *TryToCast(LogicElement *logic_element);
};
