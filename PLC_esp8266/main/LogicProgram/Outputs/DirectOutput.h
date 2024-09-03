#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class DirectOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  protected:

  public:
    explicit DirectOutput();
    ~DirectOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override;
    TvElementType GetElementType() override final;
};
