#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class LabeledLogicItem {
  protected:
    const char *label;
    uint8_t width;
    uint8_t height;

  public:
    explicit LabeledLogicItem(const char *label);
    virtual ~LabeledLogicItem();
};
