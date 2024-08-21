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

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
};
