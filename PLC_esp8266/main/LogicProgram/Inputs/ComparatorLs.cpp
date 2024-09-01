#include "LogicProgram/Inputs/ComparatorLs.h"
#include "Display/bitmaps/cmp_less_active.h"
#include "Display/bitmaps/cmp_less_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLs::ComparatorLs(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

ComparatorLs::~ComparatorLs() {
}

const Bitmap *ComparatorLs::GetCurrentBitmap() {
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

size_t ComparatorLs::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = et_ComparatorLs;
    if (!WriteRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!WriteRecord(&ref_percent04, sizeof(ref_percent04), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!WriteRecord(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t ComparatorLs::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t _ref_percent04;
    if (!ReadRecord(&_ref_percent04, sizeof(_ref_percent04), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_ref_percent04 > LogicElement::MaxValue) {
        return 0;
    }

    MapIO _io_adr;
    if (!ReadRecord(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    ref_percent04 = _ref_percent04;
    io_adr = _io_adr;
    return readed;
}