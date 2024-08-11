#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorGr : public CommonComparator {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ComparatorGr(uint16_t reference, const MapIO io_adr, InputBase *incoming_item);
    ~ComparatorGr();

    bool DoAction() override final;
};
