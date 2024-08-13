#include "LogicProgram/Inputs/ComparatorLs.h"
#include "Display/bitmaps/cmp_less_active.h"
#include "Display/bitmaps/cmp_less_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLs::ComparatorLs(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
    : CommonComparator(reference, io_adr ,incoming_item) {
}

ComparatorLs::~ComparatorLs() {
}

bool ComparatorLs::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorLs::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLs::bitmap_active;

        default:
            return &ComparatorLs::bitmap_passive;
    }
}

bool ComparatorLs::Render(uint8_t *fb) {
    bool res = CommonComparator::Render(fb);
    return res;
}