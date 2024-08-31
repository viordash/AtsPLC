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
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t RightPadding = 0;
    CommonOutput(const MapIO io_adr);
    ~CommonOutput();

    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override;
};
