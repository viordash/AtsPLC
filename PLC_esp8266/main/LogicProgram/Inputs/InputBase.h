#pragma once

#include "Display/Common.h"
#include "Display/DisplayChainItem.h"
#include "Display/LabeledLogicItem.h"
#include "LogicProgram/LogicInputElement.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputBase : public LogicInputElement, public DisplayChainItem, public LabeledLogicItem {
  protected:
    InputBase(const Controller &controller,
              const MapIO io_adr,
              const Point &incoming_point,
              LogicItemState incoming_item_state);

  public:
    InputBase(const MapIO io_adr, const InputBase &prev_item);
    virtual ~InputBase();
};
