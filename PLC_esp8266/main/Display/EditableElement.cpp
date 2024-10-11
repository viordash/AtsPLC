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
#include "EditableElement.h"

static const char *TAG_EditableElement = "EditableElement";

EditableElement::EditableElement() {
    editable_state = TEditableElementState::des_Regular;
    skip_rendering = false;
}

EditableElement::~EditableElement() {
}

void EditableElement::Select() {
    ESP_LOGD(TAG_EditableElement, "Select");

    switch (editable_state) {
        case TEditableElementState::des_Regular:
            editable_state = TEditableElementState::des_Selected;
            break;

        default:
            ESP_LOGE(TAG_EditableElement,
                     "Select, unexpected state (%u)",
                     (unsigned)editable_state);
            break;
    }
}
void EditableElement::CancelSelection() {
    ESP_LOGD(TAG_EditableElement, "CancelSelection");

    switch (editable_state) {
        case TEditableElementState::des_Selected:
            editable_state = TEditableElementState::des_Regular;
            break;

        default:
            ESP_LOGE(TAG_EditableElement,
                     "CancelSelection, unexpected state (%u)",
                     (unsigned)editable_state);
            break;
    }
}

void EditableElement::BeginEditing() {
    ESP_LOGI(TAG_EditableElement, "BeginEditing, %u", (unsigned)editable_state);

    switch (editable_state) {
        case TEditableElementState::des_Selected:
            editable_state = TEditableElementState::des_Editing;
            break;

        default:
            ESP_LOGE(TAG_EditableElement,
                     "BeginEditing, unexpected state (%u)",
                     (unsigned)editable_state);
            break;
    }
}

void EditableElement::EndEditing() {
    ESP_LOGI(TAG_EditableElement, "EndEditing, %u", (unsigned)editable_state);

    switch (editable_state) {
        case TEditableElementState::des_Editing:
            editable_state = TEditableElementState::des_Regular;
            break;

        default:
            ESP_LOGE(TAG_EditableElement,
                     "EndEditing, unexpected state (%u)",
                     (unsigned)editable_state);
            break;
    }
}

bool EditableElement::Selected() {
    return editable_state == TEditableElementState::des_Selected;
}

bool EditableElement::Render(uint8_t *fb, Point *start_point) {
    if (skip_rendering) {
        return true;
    }
    const Bitmap *bitmap;
    const int blink_timer_524ms = 0x80000;

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
            bitmap = &EditableElement::bitmap_selecting_blink_2;
            draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);
            break;

        default:
            break;
    }

    return true;
}

bool EditableElement::Editing() {
    return editable_state == TEditableElementState::des_Editing;
}

void EditableElement::SkipEditableStateRendering() {
    skip_rendering = true;
}