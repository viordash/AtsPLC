#include "LogicProgram/Inputs/ComparatorGE.h"
#include "Display/bitmaps/cmp_greate_or_equal_active.h"
#include "Display/bitmaps/cmp_greate_or_equal_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGE::ComparatorGE() : CommonComparator() {
}

ComparatorGE::ComparatorGE(uint8_t ref_percent04, const MapIO io_adr) : ComparatorGE() {
    SetReference(ref_percent04);
    SetIoAdr(io_adr);
}

ComparatorGE::~ComparatorGE() {
}

const Bitmap *ComparatorGE::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ComparatorGE::bitmap_active;

        default:
            return &ComparatorGE::bitmap_passive;
    }
}

bool ComparatorGE::CompareFunction() {
    return GetValue() >= ref_percent04;
}

TvElementType ComparatorGE::GetElementType() {
    return TvElementType::et_ComparatorGE;
}

ComparatorGE *ComparatorGE::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorGE:
            return static_cast<ComparatorGE *>(common_comparator);

        default:
            return NULL;
    }
}