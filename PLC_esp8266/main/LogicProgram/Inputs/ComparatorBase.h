#pragma once

#include "LogicProgram/Inputs/InputBase.h"
#include <stdint.h>
#include <unistd.h>

class ComparatorBase : public InputBase {
  private:
    char str_reference[16];
    int str_size;
    
  protected:
    int32_t reference;

  public:
    ComparatorBase(int32_t reference, const MapIO io_adr, InputBase &prior_item);
    ~ComparatorBase();

    void Render(uint8_t *fb) override final;
};
