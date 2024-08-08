#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/NetworkedLogicItem.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputBase : public LogicElement, public NetworkedLogicItem, public LabeledLogicItem {
  protected:
    MapIO io_adr;
    InputBase *prior_item;

    InputBase(const Controller &controller, const MapIO io_adr, const Point &incoming_point);
    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 2;
    InputBase(const MapIO io_adr, InputBase &prior_item);
    ~InputBase();

    void Render(uint8_t *fb) override;
    Point OutcomingPoint() override;
};
