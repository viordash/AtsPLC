#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class SelectableElement {
  protected:
    bool selected;

    const static Bitmap bitmap_0;
    const static Bitmap bitmap_1;

    bool Render(uint8_t *fb, Point *start_point);

  public:
    explicit SelectableElement();
    virtual ~SelectableElement();

    void ChangeSelection(bool selected);
    bool Selected();
};
