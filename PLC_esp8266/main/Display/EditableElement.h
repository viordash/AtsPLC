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

typedef enum { //
    cepi_None = 0
} TCommonEditingPropertyId;

class EditableElement {
  protected:
    TEditableElementState editable_state;
    int editing_property_id;

    const static Bitmap bitmap_selecting_blink_0;
    const static Bitmap bitmap_selecting_blink_1;
    const static Bitmap bitmap_selecting_blink_2;
    const static Bitmap bitmap_selecting_blink_3;
    const int blink_timer_524ms = 0x80000;

    bool Render(uint8_t *fb, Point *start_point);

  public:
    explicit EditableElement();
    virtual ~EditableElement();

    void Select();
    void CancelSelection();

    void BeginEditing();
    virtual void EndEditing();

    bool Selected();
    bool Editing();
};
