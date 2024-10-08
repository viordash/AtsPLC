#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonComparator::CommonComparator() : CommonInput() {
    this->ref_percent04 = 0;
}

CommonComparator::~CommonComparator() {
}

void CommonComparator::SetReference(uint8_t ref_percent04) {
    if (ref_percent04 > LogicElement::MaxValue) {
        ref_percent04 = LogicElement::MaxValue;
    }
    this->ref_percent04 = ref_percent04;
    str_size = sprintf(this->str_reference, "%d", ref_percent04);
}

bool CommonComparator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
        && this->CompareFunction()) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
    }
    return any_changes;
}

IRAM_ATTR bool
CommonComparator::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    bool res;
    res = CommonInput::Render(fb, state, start_point);

    if (!res) {
        return res;
    }
    uint8_t x_pos = start_point->x + LeftPadding + LabeledLogicItem::width + 2;
    switch (str_size) {
        case 1:
            res = draw_text_f5X7(fb, x_pos + 3, start_point->y + 2, str_reference);
            break;
        case 2:
            res = draw_text_f5X7(fb, x_pos + 0, start_point->y + 2, str_reference);
            break;
        default:
            res = draw_text_f4X7(fb, x_pos, start_point->y + 3, str_reference);
            break;
    }
    return res;
}

size_t CommonComparator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&ref_percent04, sizeof(ref_percent04), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonComparator::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t _ref_percent04;
    if (!Record::Read(&_ref_percent04, sizeof(_ref_percent04), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_ref_percent04 > LogicElement::MaxValue) {
        return 0;
    }

    MapIO _io_adr;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    ref_percent04 = _ref_percent04;
    io_adr = _io_adr;
    return readed;
}