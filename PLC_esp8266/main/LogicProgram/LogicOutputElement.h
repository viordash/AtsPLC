#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class OutputBase;
typedef void (*f_SetValue)(uint8_t);
class LogicOutputElement : public LogicElement {
  protected:
    f_SetValue SetValue = NULL;

    friend OutputBase;

  public:
    LogicOutputElement(const Controller &controller, const MapIO io_adr);
    virtual ~LogicOutputElement();
};
