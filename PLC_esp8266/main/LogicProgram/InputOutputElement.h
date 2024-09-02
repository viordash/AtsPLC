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
    explicit InputOutputElement();
    virtual ~InputOutputElement();

    void SetIoAdr(const MapIO io_adr) override final;
};
