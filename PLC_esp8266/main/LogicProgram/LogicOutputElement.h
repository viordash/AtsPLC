#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef void (*f_SetValue)(uint8_t);
class LogicOutputElement {
  protected:
    f_SetValue SetValue = NULL;

  public:
    explicit LogicOutputElement(const MapIO io_adr);
    virtual ~LogicOutputElement();
};
