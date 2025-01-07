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

ComparatorGr::ComparatorGr(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
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
    return Input->ReadValue() > ref_percent04;
}

TvElementType ComparatorGr::GetElementType() {
    return TvElementType::et_ComparatorGr;
}

ComparatorGr *ComparatorGr::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorGr:
            return static_cast<ComparatorGr *>(common_comparator);

        default:
            return NULL;
    }
}

const AllowedIO ComparatorGr::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}