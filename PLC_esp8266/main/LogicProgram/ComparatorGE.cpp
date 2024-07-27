#include "LogicProgram/ComparatorGE.h"
#include "Display/bitmaps/cmp_greate_or_equal_active.h"
#include "Display/bitmaps/cmp_greate_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGE::ComparatorGE(const MapIO io_adr, const Point &incoming_point)
    : InputBase(io_adr, incoming_point) {
}
ComparatorGE::ComparatorGE(const MapIO io_adr, InputBase &prior_item)
    : InputBase(io_adr, prior_item) {
}

ComparatorGE::~ComparatorGE() {
}

bool ComparatorGE::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorGE::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorGE::bitmap_active;

        default:
            return &ComparatorGE::bitmap_passive;
    }
}
