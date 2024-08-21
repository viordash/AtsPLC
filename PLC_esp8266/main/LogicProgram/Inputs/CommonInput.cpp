#include "LogicProgram/Inputs/CommonInput.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonInput::CommonInput(const MapIO io_adr, InputBase *incoming_item)
    : InputBase(incoming_item->controller, incoming_item->OutcomingPoint()),
      InputElement(io_adr), LabeledLogicItem(MapIONames[io_adr]) {
    this->incoming_item = incoming_item;
}

CommonInput::~CommonInput() {
}

bool CommonInput::Render(uint8_t *fb, LogicItemState prev_state) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    if (incoming_item->GetState() == LogicItemState::lisActive) {
        res &= draw_active_network(fb,
                                   incoming_point.x,
                                   incoming_point.y,
                                   LabeledLogicItem::width + LeftPadding);
    } else {
        res &= draw_passive_network(fb,
                                    incoming_point.x,
                                    incoming_point.y,
                                    LabeledLogicItem::width + LeftPadding,
                                    false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;
    res &= draw_text_f6X12(fb, x_pos, incoming_point.y - LabeledLogicItem::height, label);

    x_pos += LabeledLogicItem::width;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    if (state == LogicItemState::lisActive) {
        res &= draw_active_network(fb, x_pos, incoming_point.y, RightPadding);
    } else {
        res &= draw_passive_network(fb, x_pos, incoming_point.y, RightPadding, true);
    }
    return res;
}

Point CommonInput::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}