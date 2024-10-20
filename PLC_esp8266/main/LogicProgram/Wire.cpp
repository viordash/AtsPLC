#include "LogicProgram/Wire.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Wire::Wire() : LogicElement() {
    width = 1;
}

Wire::~Wire() {
}

bool Wire::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR bool Wire::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, width);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, width, false);
    }

    return res;
}

size_t Wire::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&width, sizeof(width), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t Wire::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint16_t _width;
    if (!Record::Read(&_width, sizeof(_width), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_width == 0
        || _width > (DISPLAY_WIDTH - (INCOME_RAIL_WIDTH + OUTCOME_RAIL_WIDTH + SCROLLBAR_WIDTH))) {
        return 0;
    }
    width = _width;
    return readed;
}

TvElementType Wire::GetElementType() {
    return TvElementType::et_Wire;
}

void Wire::SelectPrior() {
}

void Wire::SelectNext() {
}

void Wire::PageUp() {
}

void Wire::PageDown() {
}

void Wire::Change() {
    EndEditing();
}

bool Wire::EditingCompleted() {
    return true;
}