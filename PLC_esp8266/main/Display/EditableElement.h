#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

typedef enum { //
    des_Regular,
    des_Selected,
    des_Editing
} TEditableElementState;

class EditableElement {
  protected:
    TEditableElementState state;

    const static Bitmap bitmap_selecting_blink_0;
    const static Bitmap bitmap_selecting_blink_1;
    const static Bitmap bitmap_selecting_blink_2;

    bool Render(uint8_t *fb, Point *start_point);

  public:
    explicit EditableElement();
    virtual ~EditableElement();

    void ChangeSelection(bool selected);
    void ChangeEditing(bool edited);
    bool Selected();
    bool Editing();
};
