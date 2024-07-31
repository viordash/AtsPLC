#include "LogicProgram/ComparatorBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorBase::ComparatorBase(int32_t reference, const MapIO io_adr, InputBase &prior_item)
    : InputBase(io_adr, prior_item) {
    this->reference = reference;
    text_f5X7 = sprintf(this->str_reference, "%d", reference) <= 2;
}

ComparatorBase::~ComparatorBase() {
}

void ComparatorBase::Render(uint8_t *fb) {
    InputBase::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width + 2;
    if (text_f5X7) {
        draw_text_f5X7(x_pos, incoming_point.y + 2, str_reference);
    } else {
        draw_text_f4X7(x_pos, incoming_point.y + 3, str_reference);
    }
}