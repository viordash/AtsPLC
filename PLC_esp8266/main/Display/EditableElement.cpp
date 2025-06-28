#include "Display/EditableElement.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/bitmaps/element_cursor_2.h"
#include "Display/bitmaps/moving_up_down.h"
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

bool EditableElement::Render(uint8_t *fb, Point *start_point) {
    const Bitmap *bitmap = GetCursorBitmap();
    if (bitmap != NULL) {
        draw_bitmap(fb, start_point->x + 1, start_point->y + 2, bitmap);
    }

    return true;
}

const Bitmap *EditableElement::GetCursorBitmap() {
    switch (editable_state) {
        case EditableElement::ElementState::des_Selected:
            if (Blinking_50()) {
                return &EditableElement::bitmap_selecting_blink_0;
            } else {
                return &EditableElement::bitmap_selecting_blink_1;
            }

        case EditableElement::ElementState::des_Editing:
            return &EditableElement::bitmap_selecting_blink_2;

        case EditableElement::ElementState::des_Moving:
            return &EditableElement::bitmap_moving_up_down;

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

bool EditableElement::Editing() {
    return editable_state == EditableElement::ElementState::des_Editing;
}

bool EditableElement::InEditingProperty() {
    return editing_property_id != EditableElement::EditingPropertyId::cepi_None;
}

bool EditableElement::Moving() {
    return editable_state == EditableElement::ElementState::des_Moving;
}

bool EditableElement::Blinking_50() {
    const int blink_timer_us = 0x80000;
    return (esp_timer_get_time() & blink_timer_us) == blink_timer_us;
}
