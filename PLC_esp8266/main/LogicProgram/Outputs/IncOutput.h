#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/Outputs/OutputBase.h"
#include <stdint.h>
#include <unistd.h>

class IncOutput : public OutputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    IncOutput(const MapIO io_adr, InputBase &prev_item);
    ~IncOutput();

    bool DoAction() override final;
};
