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

static const char *TAG_EditableElement = "EditableElement";

EditableElement::EditableElement() {
    state = TEditableElementState::des_Regular;
}

EditableElement::~EditableElement() {
}

void EditableElement::ChangeSelection(bool selected) {
    ESP_LOGD(TAG_EditableElement, "ChangeSelection, %u", selected);

    switch (state) {
        case TEditableElementState::des_Regular:
            if (selected) {
                state = TEditableElementState::des_Selected;
            }
            break;
        case TEditableElementState::des_Selected:
            if (!selected) {
                state = TEditableElementState::des_Regular;
            }
            break;

        default:
            break;
    }
}

void EditableElement::ChangeEditing(bool edited) {
    ESP_LOGD(TAG_EditableElement, "ChangeEditing, %u", edited);

    switch (state) {
        case TEditableElementState::des_Regular:
            if (edited) {
                state = TEditableElementState::des_Editing;
            }
            break;
        case TEditableElementState::des_Editing:
            if (!edited) {
                state = TEditableElementState::des_Regular;
            }
            break;

        default:
            break;
    }
}

bool EditableElement::Selected() {
    return state == TEditableElementState::des_Selected;
}

bool EditableElement::Render(uint8_t *fb, Point *start_point) {
    const Bitmap *bitmap;
    const int blink_timer_524ms = 0x80000;

    switch (state) {
        case TEditableElementState::des_Selected:
            if (esp_timer_get_time() & blink_timer_524ms) {
                bitmap = &EditableElement::bitmap_selecting_blink_0;
            } else {
                bitmap = &EditableElement::bitmap_selecting_blink_1;
            }
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        case TEditableElementState::des_Editing:
            bitmap = &EditableElement::bitmap_selecting_blink_2;
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        default:
            break;
    }

    return true;
}

bool EditableElement::Editing() {
    return state == TEditableElementState::des_Editing;
}