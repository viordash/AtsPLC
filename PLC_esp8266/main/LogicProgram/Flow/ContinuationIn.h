#pragma once

#include "Display/display.h"
#include "LogicProgram/Flow/CommonContinuation.h"
#include <stdint.h>
#include <unistd.h>

class ContinuationIn : public CommonContinuation {
  protected:
    const uint8_t RightPadding = 1;
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override;

  public:
    explicit ContinuationIn();
    ~ContinuationIn();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    TvElementType GetElementType() override final;

    static ContinuationIn *TryToCast(LogicElement *logic_element);
};
