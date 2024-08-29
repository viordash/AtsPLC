#pragma once

#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>

class SetOutput : public CommonOutput {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    SetOutput(const MapIO io_adr, const Controller *controller);
    ~SetOutput();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
};
