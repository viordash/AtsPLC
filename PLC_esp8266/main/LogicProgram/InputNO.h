#pragma once

#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
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
    const Bitmap &GetBitmap() override;
};
