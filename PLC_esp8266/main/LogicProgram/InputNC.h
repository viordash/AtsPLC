#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "Display/NetworkedLogicItem.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputNC : public LogicItemBase, public NetworkedLogicItem, public LabeledLogicItem {
  private:
    MapIO io_adr;
    const static Bitmap bitmap_active;
    const static Bitmap bitmap_passive;

  public:
    const uint8_t LeftPadding = 2;
    const uint8_t RightPadding = 2;
    InputNC(const MapIO io_adr, const Point &incoming_point);
    ~InputNC();

    bool DoAction() override final;
    void SetOrigin() override final;
    void Render(uint8_t *fb) override final;
    Point OutcomingPoint() override final;
};
