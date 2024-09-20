#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class DesignedElement {
  protected:
    bool in_design_state;

  public:
    explicit DesignedElement();

    void BeginDesign();
    void EndDesign();
};
