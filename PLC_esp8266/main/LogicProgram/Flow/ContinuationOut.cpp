#include "LogicProgram/Flow/ContinuationOut.h"
#include "Display/bitmaps/continuation_out_active.h"
#include "Display/bitmaps/continuation_out_inactive.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ContinuationOut = "ContinuationOut";

ContinuationOut::ContinuationOut() : CommonContinuation() {
}

ContinuationOut::~ContinuationOut() {
}

IRAM_ATTR bool
ContinuationOut::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    start_point->x += LeftPadding;

    auto bitmap = GetCurrentBitmap();
    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (EditableElement::EditingPropertyId)editing_property_id
                                       == EditableElement::EditingPropertyId::cepi_None
                                && Blinking_50();
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, bitmap_top, bitmap);
    }

    start_point->x += bitmap->size.width;

    res = EditableElement::Render(fb, start_point);
    ESP_LOGD(TAG_ContinuationOut, "Render res:%u", res);
    return res;
}

const Bitmap *ContinuationOut::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ContinuationOut::bitmap_active;
        default:
            return &ContinuationOut::bitmap_passive;
    }
}

TvElementType ContinuationOut::GetElementType() {
    return TvElementType::et_ContinuationOut;
}

ContinuationOut *ContinuationOut::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ContinuationOut:
            return static_cast<ContinuationOut *>(logic_element);

        default:
            return NULL;
    }
}