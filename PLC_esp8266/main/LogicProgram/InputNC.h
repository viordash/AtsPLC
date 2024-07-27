#pragma once

#include "LogicProgram/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class InputNC : public InputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    InputNC(const MapIO io_adr, const Point &incoming_point);
    ~InputNC();

    bool DoAction() override final;
};
