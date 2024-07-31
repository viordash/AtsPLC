#pragma once

#include "LogicProgram/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class InputRail : public InputBase {
  private:
    uint8_t network_number;
    const Bitmap *GetCurrentBitmap() override final;

  public:
    explicit InputRail(uint8_t network_number);
    ~InputRail();

    bool DoAction() override final;
    void Render(uint8_t *fb) override final;
    Point OutcomingPoint() override final;
};
