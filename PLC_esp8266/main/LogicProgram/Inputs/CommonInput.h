#pragma once

#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class CommonInput : public InputBase, public InputElement, public LabeledLogicItem {
  protected:
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 0;
    CommonInput(const MapIO io_adr, const Controller *controller, const Point &incoming_point);
    virtual ~CommonInput();

    bool Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override;
    Point OutcomingPoint() override final;
};
