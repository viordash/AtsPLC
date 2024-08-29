#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class CommonOutput : public LogicElement, public InputOutputElement, public LabeledLogicItem {
  protected:
    const Controller *controller;
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t RightPadding = 0;
    CommonOutput(const MapIO io_adr, const Controller *controller);
    ~CommonOutput();

    bool Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override;
    Point OutcomingPoint();
};
