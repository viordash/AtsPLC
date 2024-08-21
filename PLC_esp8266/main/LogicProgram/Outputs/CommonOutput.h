#pragma once

#include "Display/Common.h"
#include "Display/DisplayChainItem.h"
#include "Display/LabeledLogicItem.h"
#include "Display/display.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class CommonOutput : public LogicElement,
                     public InputOutputElement,
                     public DisplayChainItem,
                     public LabeledLogicItem {
  protected:
    InputBase *incoming_item;
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t RightPadding = 0;
    CommonOutput(const MapIO io_adr, InputBase *incoming_item);
    ~CommonOutput();

    bool Render(uint8_t *fb, LogicItemState prev_state) override;
    Point OutcomingPoint() override;
};
