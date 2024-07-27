#pragma once

#include "LogicProgram/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class InputNO : public InputBase {
  private:
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

    const Bitmap *GetCurrentBitmap() override final;

  public:
    InputNO(const MapIO io_adr, const Point &incoming_point);
    InputNO(const MapIO io_adr, InputBase &prior_item);
    ~InputNO();

    bool DoAction() override final;
};
