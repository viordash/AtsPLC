#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class InputNC : public CommonInput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    InputNC(const MapIO io_adr, InputBase *incoming_item);
    ~InputNC();

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
};
