#include "LogicProgram/Inputs/ComparatorLs.h"
#include "Display/bitmaps/cmp_less_active.h"
#include "Display/bitmaps/cmp_less_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLs::ComparatorLs() : CommonComparator() {
}

ComparatorLs::ComparatorLs(uint8_t ref_percent04, const MapIO io_adr) : ComparatorLs() {
    SetReference(ref_percent04);
    SetIoAdr(io_adr);
}

ComparatorLs::~ComparatorLs() {
}

const Bitmap *ComparatorLs::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLs::bitmap_active;

        default:
            return &ComparatorLs::bitmap_passive;
    }
}

bool ComparatorLs::CompareFunction() {
    return GetValue() < ref_percent04;
}

TvElementType ComparatorLs::GetElementType() {
    return TvElementType::et_ComparatorLs;
}