#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class LabeledLogicItem {
  protected:
    const char *label;
    uint8_t label_width;

    void SetLabel(const char *label);

  public:
    const char *GetLabel();
};
