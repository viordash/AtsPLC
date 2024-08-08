#pragma once

#include "Display/Common.h"
#include "LogicProgram/AddressableLogicElement.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <functional>
#include <stdint.h>
#include <unistd.h>

class LogicInputElement : public AddressableLogicElement {
  protected:
    std::function<uint8_t(void)> GetValue;

  public:
    LogicItemState state = LogicItemState::lisPassive;

    LogicInputElement(const Controller &controller, const MapIO io_adr);
    virtual ~LogicInputElement();
};
