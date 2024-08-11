#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller, const MapIO io_adr, const Point &incoming_point)
    : LogicInputElement(controller, io_adr), ChainItem(incoming_point),
      LabeledLogicItem(MapIONames[io_adr]) {
    this->prev_item = NULL;
}

InputBase::InputBase(const MapIO io_adr, InputBase &prev_item)
    : LogicInputElement(prev_item.controller, io_adr), ChainItem(prev_item.OutcomingPoint()),
      LabeledLogicItem(MapIONames[io_adr]) {
    this->prev_item = &prev_item;
}

InputBase::~InputBase() {
}

bool InputBase::Render(uint8_t *fb) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    LogicItemState prev_item_state = prev_item != NULL ? prev_item->state : state;
    if (prev_item_state == LogicItemState::lisActive) {
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

Point InputBase::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}