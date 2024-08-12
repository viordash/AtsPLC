#include "LogicProgram/Inputs/ComparatorGr.h"
#include "Display/bitmaps/cmp_greate_active.h"
#include "Display/bitmaps/cmp_greate_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGr::ComparatorGr(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
    : CommonComparator(reference, io_adr, incoming_item) {
}

ComparatorGr::~ComparatorGr() {
}

bool ComparatorGr::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorGr::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorGr::bitmap_active;

        default:
            return &ComparatorGr::bitmap_passive;
    }
}

bool ComparatorGr::Render(uint8_t *fb) {
    if (!require_render) {
        return true;
    }
    bool res = CommonComparator::Render(fb);
    require_render = false;
    return res;
}