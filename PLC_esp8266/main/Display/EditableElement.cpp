#include "Display/EditableElement.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/bitmaps/element_cursor_2.h"
#include "Display/display.h"
#include "EditableElement.h"
#include "LogicProgram/Controller.h"
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
    const Bitmap *bitmap;

    switch (editable_state) {
        case EditableElement::ElementState::des_Selected:
            if (Blinking_50()) {
                bitmap = &EditableElement::bitmap_selecting_blink_0;
            } else {
                bitmap = &EditableElement::bitmap_selecting_blink_1;
            }
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        case EditableElement::ElementState::des_Editing:
            bitmap = &EditableElement::bitmap_selecting_blink_2;
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        default:
            break;
    }

    return true;
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
