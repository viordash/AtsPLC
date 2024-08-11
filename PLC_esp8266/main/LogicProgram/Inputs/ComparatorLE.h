#pragma once

#include "LogicProgram/Inputs/ComparatorBase.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorLE : public ComparatorBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ComparatorLE(uint16_t reference, const MapIO io_adr, InputBase *incoming_item);
    ~ComparatorLE();

    bool DoAction() override final;
};
