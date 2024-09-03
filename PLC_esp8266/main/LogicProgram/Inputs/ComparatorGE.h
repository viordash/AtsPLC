#pragma once

#include "LogicProgram/Inputs/CommonComparator.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorGE : public CommonComparator {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;
    bool CompareFunction() override final;

  public:
    ComparatorGE();
    ~ComparatorGE();
    TvElementType GetElementType() override final;
};
