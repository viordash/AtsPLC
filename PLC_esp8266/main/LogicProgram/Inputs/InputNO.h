#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class InputNO : public CommonInput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    explicit InputNO(const MapIO io_adr);
    ~InputNO();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
};
