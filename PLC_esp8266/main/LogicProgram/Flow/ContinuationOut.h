#pragma once

#include "Display/display.h"
#include "LogicProgram/Flow/CommonContinuation.h"
#include <stdint.h>
#include <unistd.h>

class ContinuationOut : public CommonContinuation {
  protected:
    const uint8_t LeftPadding = 6;
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override;

  public:
    explicit ContinuationOut();
    ~ContinuationOut();

    ActionStatus DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override;

    TvElementType GetElementType() override final;

    static ContinuationOut *TryToCast(LogicElement *logic_element);
};
