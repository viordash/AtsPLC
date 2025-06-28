#include "LogicProgram/Flow/ContinuationIn.h"
#include "Display/bitmaps/continuation_in_active.h"
#include "Display/bitmaps/continuation_in_inactive.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ContinuationIn = "ContinuationIn";

ContinuationIn::ContinuationIn() : CommonContinuation() {
}

ContinuationIn::~ContinuationIn() {
}

IRAM_ATTR bool
ContinuationIn::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    start_point->x -= RightPadding;

    auto bitmap = GetCurrentBitmap();
    start_point->x -= bitmap->size.width;

    auto cursor_width = GetCursorWidth();
    start_point->x -= cursor_width;

    res = EditableElement::Render(fb, start_point);
    if (!res) {
        return res;
    }
    start_point->x += cursor_width;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (EditableElement::EditingPropertyId)editing_property_id
                                       == EditableElement::EditingPropertyId::cepi_None
                                && Blinking_50();
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }
    ESP_LOGD(TAG_ContinuationIn, "Render res:%u", res);
    return res;
}

const Bitmap *ContinuationIn::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ContinuationIn::bitmap_active;
        default:
            return &ContinuationIn::bitmap_passive;
    }
}

TvElementType ContinuationIn::GetElementType() {
    return TvElementType::et_ContinuationIn;
}

ContinuationIn *ContinuationIn::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ContinuationIn:
            return static_cast<ContinuationIn *>(logic_element);

        default:
            return NULL;
    }
}