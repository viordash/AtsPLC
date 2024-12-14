#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/ControllerInput.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef uint8_t (*f_GetValue)(void);

class InputElement {
  protected:
    MapIO io_adr;
    f_GetValue GetValue;

  public:
    InputElement();
    virtual void SetIoAdr(const MapIO io_adr);
    MapIO GetIoAdr();

    ControllerInput *Input;

    static InputElement *TryToCast(LogicElement *logic_element);
};
