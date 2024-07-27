#pragma once

#include "Display/Common.h"
#include "Display/LabeledLogicItem.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputNO : public LogicItemBase, public LabeledLogicItem {
  private:
    MapIO io_adr;

  public:
    InputNO(const MapIO io_adr, const Point &location);
    ~InputNO();

    bool DoAction() override;
    void Render(uint8_t *fb) override;
};
