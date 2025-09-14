#include "Display/EditableElement.h"
#include "Display/bitmaps/copy_cursor_0.h"
#include "Display/bitmaps/copy_cursor_1.h"
#include "Display/bitmaps/delete_cursor_0.h"
#include "Display/bitmaps/delete_cursor_1.h"
#include "Display/bitmaps/disable_cursor_0.h"
#include "Display/bitmaps/disable_cursor_1.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/bitmaps/element_cursor_2.h"
#include "Display/bitmaps/moving_up_down_0.h"
#include "Display/bitmaps/moving_up_down_1.h"
#include "Display/display.h"
#include "EditableElement.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EditableElement::EditableElement() {
    editable_state = EditableElement::ElementState::des_Regular;
    editing_property_id = EditingPropertyId::cepi_None;
}

EditableElement::~EditableElement() {
}

void EditableElement::Select() {
    editable_state = EditableElement::ElementState::des_Selected;
}
void EditableElement::CancelSelection() {
    editable_state = EditableElement::ElementState::des_Regular;
}

void EditableElement::BeginEditing() {
    editable_state = EditableElement::ElementState::des_Editing;
    editing_property_id = EditingPropertyId::cepi_None;
}
void EditableElement::EndEditing() {
    editable_state = EditableElement::ElementState::des_Regular;
}

bool EditableElement::Selected() {
    return editable_state == EditableElement::ElementState::des_Selected;
}

bool EditableElement::Render(FrameBuffer *fb, Point *start_point) {
    const Bitmap *bitmap = GetCursorBitmap();
    if (bitmap != NULL) {
        draw_bitmap(fb, start_point->x + 1, start_point->y + 2, bitmap);
    }

    return true;
}

const Bitmap *EditableElement::GetCursorBitmap() {
    switch (editable_state) {
        case EditableElement::ElementState::des_Selected:
            return Blinking_50() ? &bitmap_selecting_blink_0 : &bitmap_selecting_blink_1;

        case EditableElement::ElementState::des_Editing:
            return &bitmap_selecting_blink_2;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            return Blinking_50() ? &bitmap_moving_up_down_0 : &bitmap_moving_up_down_1;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            return Blinking_50() ? &bitmap_copy_cursor_0 : &bitmap_copy_cursor_0;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            return Blinking_50() ? &bitmap_delete_cursor_0 : &bitmap_delete_cursor_1;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            return Blinking_50() ? &bitmap_disable_cursor_0 : &bitmap_disable_cursor_1;

        case EditableElement::ElementState::des_Moving:
            return &bitmap_moving_up_down_0;

        case EditableElement::ElementState::des_Copying:
            return &bitmap_copy_cursor_0;

        case EditableElement::ElementState::des_Deleting:
            return &bitmap_delete_cursor_0;

        case EditableElement::ElementState::des_Disabling:
            return &bitmap_disable_cursor_0;

        default:
            return NULL;
    }
}

uint16_t EditableElement::GetCursorWidth() {
    const Bitmap *bitmap = GetCursorBitmap();
    if (bitmap != NULL) {
        return bitmap->size.width;
    }
    return 0;
}

EditableElement::ElementState EditableElement::GetEditable_state() {
    return editable_state;
}

bool EditableElement::Editing() {
    return editable_state == EditableElement::ElementState::des_Editing;
}

bool EditableElement::InEditingProperty() {
    return editing_property_id != EditableElement::EditingPropertyId::cepi_None;
}

bool EditableElement::Blinking_50() {
    const int blink_timer_us = 0x80000;
    return (esp_timer_get_time() & blink_timer_us) == blink_timer_us;
}
