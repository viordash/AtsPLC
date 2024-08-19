#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include <stdint.h>
#include <unistd.h>

class CommonComparator : public CommonInput {
  private:
    char str_reference[5];
    int str_size;

  protected:
    uint8_t ref_percent04;

    virtual bool CompareFunction() = 0;

  public:
    CommonComparator(uint8_t ref_percent04, const MapIO io_adr, InputBase *incoming_item);
    ~CommonComparator();

    bool DoAction(bool prev_changed) override;
    bool Render(uint8_t *fb) override;
};
