#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/DisplayChainItem.h"
#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/LogicOutputElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class OutputBase : public LogicOutputElement, public DisplayChainItem, public LabeledLogicItem {
  protected:

    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 7;
    const uint8_t RightPadding = 0;
    OutputBase(const MapIO io_adr, InputBase &prev_item);
    ~OutputBase();

    bool Render(uint8_t *fb) override;
    Point OutcomingPoint() override;
};
