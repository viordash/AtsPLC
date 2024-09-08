#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorGE : public CommonComparator {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    bool CompareFunction() override final;

  public:
    ComparatorGE();
    ComparatorGE(uint8_t ref_percent04, const MapIO io_adr);
    ~ComparatorGE();
    TvElementType GetElementType() override final;
};
