#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/InputElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef void (*f_SetValue)(uint8_t);
class InputOutputElement : public InputElement {
  protected:
    f_SetValue SetValue = NULL;

  public:
    explicit InputOutputElement(const MapIO io_adr);
    virtual ~InputOutputElement();
};
