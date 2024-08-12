#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorEq : public CommonComparator {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ComparatorEq(uint16_t reference, const MapIO io_adr, InputBase *prev_item);
    ~ComparatorEq();

    bool DoAction() override final;
    bool Render(uint8_t *fb) override final;
};
