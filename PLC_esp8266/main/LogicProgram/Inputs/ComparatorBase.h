#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorBase : public InputBase {
  private:
    char str_reference[16];
    bool text_f5X7 = false;
    
  protected:
    int32_t reference;

  public:
    ComparatorBase(int32_t reference, const MapIO io_adr, InputBase &prior_item);
    ~ComparatorBase();

    void Render(uint8_t *fb) override final;
};
