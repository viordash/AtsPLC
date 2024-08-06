#include "LogicProgram/Inputs/ComparatorLe.h"
#include "Display/bitmaps/cmp_less_active.h"
#include "Display/bitmaps/cmp_less_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLe::ComparatorLe(uint16_t reference, const MapIO io_adr, InputBase &prior_item)
    : ComparatorBase(reference, io_adr, prior_item) {
}

ComparatorLe::~ComparatorLe() {
}

bool ComparatorLe::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ComparatorLe::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLe::bitmap_active;

        default:
            return &ComparatorLe::bitmap_passive;
    }
}