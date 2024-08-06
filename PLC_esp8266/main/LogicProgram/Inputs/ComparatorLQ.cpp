#include "LogicProgram/Inputs/ComparatorLQ.h"
#include "Display/bitmaps/cmp_less_or_equal_active.h"
#include "Display/bitmaps/cmp_less_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLQ::ComparatorLQ(uint16_t reference, const MapIO io_adr, InputBase &prior_item)
    : ComparatorBase(reference, io_adr, prior_item) {
}

ComparatorLQ::~ComparatorLQ() {
}

bool ComparatorLQ::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorLQ::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLQ::bitmap_active;

        default:
            return &ComparatorLQ::bitmap_passive;
    }
}