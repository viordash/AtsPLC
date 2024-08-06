#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include <stdint.h>
#include <unistd.h>

class MapIOIndicator : public DisplayItemBase {
  protected:
    Point incoming_point;
    const char *name;
    uint8_t progress;
    static const uint8_t text_width = 6;
    static const uint8_t text_height = 12;
    static const uint8_t name_size = 2;
    static const uint8_t margin = 1;
    static const uint8_t delimeter_width = 1;

  public:
    explicit MapIOIndicator(const Point &incoming_point, const char *name, uint8_t progress);
    virtual ~MapIOIndicator();

    void Render(uint8_t *fb) override final;
    void SetOrigin() override final;
    static uint8_t GetWidth();
    static uint8_t GetHeight();
};
