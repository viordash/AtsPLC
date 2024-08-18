#pragma once

#include "Display/LabeledLogicItem.h"
#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/InputElement.h"
#include <stdint.h>
#include <unistd.h>

class CommonInput : public InputBase, public InputElement, public LabeledLogicItem {
  protected:
    InputBase *incoming_item;
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 0;
    CommonInput(const MapIO io_adr, InputBase *incoming_item);
    virtual ~CommonInput();

    bool Render(uint8_t *fb) override;
    Point OutcomingPoint() override final;
};
