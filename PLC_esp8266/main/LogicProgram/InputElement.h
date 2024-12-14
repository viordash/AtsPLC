#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/ControllerBaseInput.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class InputElement {
  protected:
  public:
    InputElement();
    virtual void SetIoAdr(const MapIO io_adr);
    MapIO GetIoAdr();

    ControllerBaseInput *Input;

    static InputElement *TryToCast(LogicElement *logic_element);
};
