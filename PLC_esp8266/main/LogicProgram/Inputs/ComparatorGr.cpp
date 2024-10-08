#include "LogicProgram/Inputs/ComparatorGr.h"
#include "Display/bitmaps/cmp_greate_active.h"
#include "Display/bitmaps/cmp_greate_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGr::ComparatorGr() : CommonComparator() {
}

ComparatorGr::ComparatorGr(uint8_t ref_percent04, const MapIO io_adr) : ComparatorGr() {
    SetReference(ref_percent04);
    SetIoAdr(io_adr);
}

ComparatorGr::~ComparatorGr() {
}

const Bitmap *ComparatorGr::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorGr::bitmap_active;

        default:
            return &ComparatorGr::bitmap_passive;
    }
}

bool ComparatorGr::CompareFunction() {
    return GetValue() > ref_percent04;
}

TvElementType ComparatorGr::GetElementType() {
    return TvElementType::et_ComparatorGr;
}