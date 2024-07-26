#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class LabeledLogicItem : public DisplayItemBase {
  protected:
    const char *label;

  public:
    LabeledLogicItem(const char *label, const Point &location);
    virtual ~LabeledLogicItem();
};
