#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class EditableElement {

  public:
    typedef enum { //
        des_Regular,
        des_Selected,
        des_Editing
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
    const static Bitmap bitmap_selecting_blink_3;

    bool Render(uint8_t *fb, Point *start_point);

    bool Blinking_50();
    bool Blinking_10();

  public:
    explicit EditableElement();
    virtual ~EditableElement();

    void Select();
    void CancelSelection();

    virtual void BeginEditing();
    virtual void EndEditing();

    bool Selected();
    bool Editing();
    bool InEditingProperty();

    virtual void SelectPrior() = 0;
    virtual void SelectNext() = 0;
    virtual void PageUp() = 0;
    virtual void PageDown() = 0;
    virtual void Change() = 0;
};
