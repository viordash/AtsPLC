#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include "LogicProgram/Outputs/OutputBase.h"
#include <stdint.h>
#include <unistd.h>

class DecOutput : public OutputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    DecOutput(const MapIO io_adr, InputBase &prior_item);
    ~DecOutput();

    bool DoAction() override final;
};
