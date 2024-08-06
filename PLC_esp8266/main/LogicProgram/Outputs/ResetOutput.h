#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/Outputs/OutputBase.h"
#include <stdint.h>
#include <unistd.h>

class ResetOutput : public OutputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ResetOutput(const MapIO io_adr, InputBase &prior_item);
    ~ResetOutput();

    bool DoAction() override final;
};
