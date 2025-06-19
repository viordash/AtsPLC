#include "LogicProgram/Inputs/ComparatorNe.h"
#include "Display/bitmaps/cmp_not_equal_active.h"
#include "Display/bitmaps/cmp_not_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorNe::ComparatorNe() : CommonComparator() {
}

ComparatorNe::ComparatorNe(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorNe::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorNe::bitmap_active;

        default:
            return &ComparatorNe::bitmap_passive;
    }
}

bool ComparatorNe::CompareFunction() {
    return Input->ReadValue() != ref_percent04;
}

TvElementType ComparatorNe::GetElementType() {
    return TvElementType::et_ComparatorNe;
}

ComparatorNe *ComparatorNe::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorNe:
            return static_cast<ComparatorNe *>(common_comparator);

        default:
            return NULL;
    }
}

const AllowedIO ComparatorNe::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}