#pragma once

#include "LogicProgram/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorGE : public InputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    ComparatorGE(const MapIO io_adr, const Point &incoming_point);
    ComparatorGE(const MapIO io_adr, InputBase &prior_item);
    ~ComparatorGE();

    bool DoAction() override final;
};
