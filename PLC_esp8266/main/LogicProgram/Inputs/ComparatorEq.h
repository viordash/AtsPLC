#pragma once

#include "LogicProgram/Inputs/ComparatorBase.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorEq : public ComparatorBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ComparatorEq(uint16_t reference, const MapIO io_adr, InputBase *prev_item);
    ~ComparatorEq();

    bool DoAction() override final;
};
