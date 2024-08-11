#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef uint8_t (*f_GetValue)(void);

class LogicInputElement {
  protected:
    f_GetValue GetValue;

  public:
    LogicItemState state = LogicItemState::lisPassive;

    explicit LogicInputElement(const MapIO io_adr);
    virtual ~LogicInputElement();
};
