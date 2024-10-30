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

ComparatorLs::ComparatorLs(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
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

ComparatorLs *ComparatorLs::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorLs:
            return static_cast<ComparatorLs *>(common_comparator);

        default:
            return NULL;
    }
}

const AllowedIO ComparatorLs::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}