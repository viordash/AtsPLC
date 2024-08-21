#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class ResetOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ResetOutput(const MapIO io_adr, InputBase *incoming_item);
    ~ResetOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
};
