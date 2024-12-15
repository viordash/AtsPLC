#include "LogicProgram/Inputs/ComparatorLE.h"
#include "Display/bitmaps/cmp_less_or_equal_active.h"
#include "Display/bitmaps/cmp_less_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLE::ComparatorLE() : CommonComparator() {
}

ComparatorLE::ComparatorLE(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorLE::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorLE::bitmap_active;

        default:
            return &ComparatorLE::bitmap_passive;
    }
}

bool ComparatorLE::CompareFunction() {
    return Input->GetValue() <= ref_percent04;
}

TvElementType ComparatorLE::GetElementType() {
    return TvElementType::et_ComparatorLE;
}

ComparatorLE *ComparatorLE::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorLE:
            return static_cast<ComparatorLE *>(common_comparator);

        default:
            return NULL;
    }
}

const AllowedIO ComparatorLE::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}