#include "LogicProgram/Inputs/CommonComparator.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonComparator::CommonComparator(uint8_t ref_percent04, const MapIO io_adr)
    : CommonInput(io_adr) {
    if (ref_percent04 > LogicElement::MaxValue) {
        ref_percent04 = LogicElement::MaxValue;
    }
    this->ref_percent04 = ref_percent04;
    str_size = sprintf(this->str_reference, "%d", ref_percent04);
}

CommonComparator::~CommonComparator() {
}

bool CommonComparator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_changed;
    bool any_changes = false;
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

bool CommonComparator::Render(uint8_t *fb,
                              LogicItemState prev_elem_state,
                              Point *start_point) {
    (void)prev_elem_state;
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