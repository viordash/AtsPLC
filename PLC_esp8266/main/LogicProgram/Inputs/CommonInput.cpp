#include "LogicProgram/Inputs/CommonInput.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonInput::CommonInput() : LogicElement(), InputElement(), LabeledLogicItem(MapIONames[io_adr]) {
}

CommonInput::~CommonInput() {
}

bool CommonInput::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb,
                                  start_point->x,
                                  start_point->y,
                                  LabeledLogicItem::width + LeftPadding);
    } else {
        res = draw_passive_network(fb,
                                   start_point->x,
                                   start_point->y,
                                   LabeledLogicItem::width + LeftPadding,
                                   false);
    }
    if (!res) {
        return res;
    }

    start_point->x += LeftPadding;
    res = draw_text_f6X12(fb, start_point->x, start_point->y - LabeledLogicItem::height, label);
    if (!res) {
        return res;
    }

    start_point->x += LabeledLogicItem::width;
    draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);

    start_point->x += bitmap->size.width;
    return res;
}