#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class AddressableLogicElement : public LogicElement {
  protected:
    const char *name;

  public:
    AddressableLogicElement(const Controller &controller, const MapIO addr);
    virtual ~AddressableLogicElement();
};
