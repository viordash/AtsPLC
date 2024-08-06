#include "LogicProgram/Inputs/ComparatorLE.h"
#include "Display/bitmaps/cmp_less_or_equal_active.h"
#include "Display/bitmaps/cmp_less_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLE::ComparatorLE(uint16_t reference, const MapIO io_adr, InputBase &prior_item)
    : ComparatorBase(reference, io_adr, prior_item) {
}

ComparatorLE::~ComparatorLE() {
}

bool ComparatorLE::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorLE::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLE::bitmap_active;

        default:
            return &ComparatorLE::bitmap_passive;
    }
}