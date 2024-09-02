#pragma once

#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class ResetOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  protected:
    TvElementType GetElementType() override final;

  public:
    explicit ResetOutput();
    ~ResetOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
};
