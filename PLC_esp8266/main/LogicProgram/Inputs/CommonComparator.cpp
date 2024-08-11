#include "LogicProgram/Inputs/CommonComparator.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonComparator::CommonComparator(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
    : CommonInput(io_adr, incoming_item) {
    if (reference > 999) {
        reference = 999;
    }
    this->reference = reference;
    this->incoming_item = incoming_item;
    str_size = sprintf(this->str_reference, "%d", reference);
}

CommonComparator::~CommonComparator() {
}

bool CommonComparator::Render(uint8_t *fb) {
    bool res;
    res = CommonInput::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width + 2;
    switch (str_size) {
        case 1:
            res &= draw_text_f5X7(fb, x_pos + 3, incoming_point.y + 2, str_reference);
            break;
        case 2:
            res &= draw_text_f5X7(fb, x_pos + 0, incoming_point.y + 2, str_reference);
            break;
        default:
            res &= draw_text_f4X7(fb, x_pos, incoming_point.y + 3, str_reference);
            break;
    }
    return res;
}