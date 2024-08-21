#pragma once

#include "Display/Common.h"
#include "Display/display.h"
#include "Display/DisplayChainItem.h"
#include "Display/LabeledLogicItem.h"
#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/StatefulElement.h"
#include "LogicProgram/InputOutputElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class CommonOutput : public StatefulElement,
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

    bool Render(uint8_t *fb, LogicItemState state) override;
    Point OutcomingPoint() override;
};
