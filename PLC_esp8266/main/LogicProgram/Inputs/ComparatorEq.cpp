#include "LogicProgram/Inputs/ComparatorEq.h"
#include "Display/bitmaps/cmp_equal_active.h"
#include "Display/bitmaps/cmp_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorEq::ComparatorEq(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
    : CommonComparator(reference, io_adr, incoming_item) {
}

ComparatorEq::~ComparatorEq() {
}

bool ComparatorEq::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorEq::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorEq::bitmap_active;

        default:
            return &ComparatorEq::bitmap_passive;
    }
}

bool ComparatorEq::Render(uint8_t *fb) {
    if (!require_render) {
        return true;
    }
    bool res = CommonComparator::Render(fb);
    require_render = false;
    return res;
}