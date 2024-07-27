#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class LabeledLogicItem : public DisplayItemBase {
  protected:
    const char *label;
    uint8_t width;
    uint8_t height;

  public:
    LabeledLogicItem(const char *label, const Point &location);
    virtual ~LabeledLogicItem();

    void Render(uint8_t *fb) override;
};
