#include "LogicProgram/Inputs/ComparatorEQ.h"
#include "Display/bitmaps/cmp_equal_active.h"
#include "Display/bitmaps/cmp_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorEQ::ComparatorEQ(uint16_t reference, const MapIO io_adr, InputBase &prior_item)
    : ComparatorBase(reference, io_adr, prior_item) {
}

ComparatorEQ::~ComparatorEQ() {
}

bool ComparatorEQ::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorEQ::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorEQ::bitmap_active;

        default:
            return &ComparatorEQ::bitmap_passive;
    }
}