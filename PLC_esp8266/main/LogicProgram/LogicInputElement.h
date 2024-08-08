#pragma once

#include "Display/Common.h"
#include "LogicProgram/AddressableLogicElement.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef uint8_t (*f_GetValue)(void);
class LogicInputElement : public AddressableLogicElement {
  protected:
    f_GetValue GetValue = NULL;

  public:
    LogicItemState state = LogicItemState::lisPassive;

    LogicInputElement(const Controller &controller, const MapIO io_adr);
    virtual ~LogicInputElement();
};
