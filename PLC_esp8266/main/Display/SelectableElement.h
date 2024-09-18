#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class SelectableElement {
  protected:
    bool selected;

    bool Render(uint8_t *fb, Point *start_point);

  public:
    explicit SelectableElement();
    virtual ~SelectableElement();

    void ChangeSelection(bool selected);
    bool Selected();
};
