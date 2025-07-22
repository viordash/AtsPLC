#pragma once

#include "Display/Common.h"
#include <stdint.h>
#include <unistd.h>

class EditableElement {

  public:
    typedef enum { //
        des_Regular,
        des_Selected,
        des_Editing,
        des_AdvancedSelectMove,
        des_AdvancedSelectCopy,
        des_AdvancedSelectDelete,
        des_Moving,
        des_Copying,
        des_Deleting
    } ElementState;

    typedef enum { //
        cepi_None = 0
    } EditingPropertyId;

  protected:
    EditableElement::ElementState editable_state;
    int editing_property_id;

    const static Bitmap bitmap_selecting_blink_0;
    const static Bitmap bitmap_selecting_blink_1;
    const static Bitmap bitmap_selecting_blink_2;
    const static Bitmap bitmap_moving_up_down_0;
    const static Bitmap bitmap_moving_up_down_1;
    const static Bitmap bitmap_copy_cursor_0;
    const static Bitmap bitmap_copy_cursor_1;
    const static Bitmap bitmap_delete_cursor_0;
    const static Bitmap bitmap_delete_cursor_1;

    bool Render(uint8_t *fb, Point *start_point);

    bool Blinking_50();
    const Bitmap *GetCursorBitmap();
    uint16_t GetCursorWidth();

  public:
    explicit EditableElement();
    virtual ~EditableElement();

    void Select();
    void CancelSelection();

    virtual void BeginEditing();
    virtual void EndEditing();

    EditableElement::ElementState GetEditable_state();
    bool Selected();
    bool Editing();
    bool InEditingProperty();

    virtual void SelectPrior() = 0;
    virtual void SelectNext() = 0;
    virtual void PageUp() = 0;
    virtual void PageDown() = 0;
    virtual void Change() = 0;
    virtual void Option() = 0;
};
