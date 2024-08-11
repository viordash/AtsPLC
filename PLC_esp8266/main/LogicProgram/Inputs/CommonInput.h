#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class CommonInput : public InputBase {
  protected:
    InputBase *prev_item;

    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 2;
    CommonInput(const MapIO io_adr, InputBase &prev_item);
    ~CommonInput();

    bool Render(uint8_t *fb) override;
    Point OutcomingPoint() override final;
};
