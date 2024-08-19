#pragma once

#include "Display/Common.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/LogicItemState.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class StatefulElement : public LogicElement {
  protected:
    LogicItemState state;


  public:
    explicit StatefulElement(const Controller *controller);
    virtual ~StatefulElement();

    LogicItemState GetState();
};
