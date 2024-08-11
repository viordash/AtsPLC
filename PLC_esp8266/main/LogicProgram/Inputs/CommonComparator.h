#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class CommonComparator : public CommonInput {
  private:
    char str_reference[5];
    int str_size;

  protected:
    uint16_t reference;

  public:
    CommonComparator(uint16_t reference, const MapIO io_adr, InputBase *incoming_item);
    ~CommonComparator();

    bool Render(uint8_t *fb) override final;
};
