#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/NetworkedLogicItem.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputBase : public LogicItemBase, public NetworkedLogicItem, public LabeledLogicItem {
  protected:
    MapIO io_adr;
    InputBase *prior_item;

    virtual const Bitmap *GetCurrentBitmap() = 0;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 2;
    InputBase(const MapIO io_adr, const Point &incoming_point);
    InputBase(const MapIO io_adr, InputBase &prior_item);
    ~InputBase();

    void SetOrigin() override final;
    void Render(uint8_t *fb) override final;
    Point OutcomingPoint() override final;
};
