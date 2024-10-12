#include "Display/EditableElement.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/bitmaps/element_cursor_2.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EditableElement::EditableElement() {
    editable_state = TEditableElementState::des_Regular;
}

EditableElement::~EditableElement() {
}

void EditableElement::Select() {
    editable_state = TEditableElementState::des_Selected;
}
void EditableElement::CancelSelection() {
    editable_state = TEditableElementState::des_Regular;
}

void EditableElement::BeginEditing() {
    editable_state = TEditableElementState::des_Editing;
    editing_property_id = 0;
}
void EditableElement::EndEditing() {
    editable_state = TEditableElementState::des_Regular;
}

bool EditableElement::Selected() {
    return editable_state == TEditableElementState::des_Selected;
}

bool EditableElement::Render(uint8_t *fb, Point *start_point) {
    const Bitmap *bitmap;

    switch (editable_state) {
        case TEditableElementState::des_Selected:
            if (esp_timer_get_time() & blink_timer_524ms) {
                bitmap = &EditableElement::bitmap_selecting_blink_0;
            } else {
                bitmap = &EditableElement::bitmap_selecting_blink_1;
            }
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        case TEditableElementState::des_Editing:
            if ((TCommonEditingPropertyId)editing_property_id
                == TCommonEditingPropertyId::cepi_None) {
                bitmap = &EditableElement::bitmap_selecting_blink_2;
                draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            }
            break;

        default:
            break;
    }

    return true;
}

bool EditableElement::Editing() {
    return editable_state == TEditableElementState::des_Editing;
}