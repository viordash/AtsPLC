#pragma once

#include "Display/DisplayItemBase.h"
#include "Display/Location.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputNO : public LogicItemBase, public DisplayItemBase {
  private:
    MapIO io_adr;

  public:
    InputNO(const MapIO io_adr, const Point &location);
    ~InputNO();

    bool DoAction() override;
    uint8_t *GetBitmap() override;
};
