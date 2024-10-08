#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorGr : public CommonComparator {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap(LogicItemState state) override final;
    bool CompareFunction() override final;

  public:
    ComparatorGr();
    ComparatorGr(uint8_t ref_percent04, const MapIO io_adr);
    ~ComparatorGr();
    TvElementType GetElementType() override final;
};
