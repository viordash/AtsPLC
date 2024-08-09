#include "LogicProgram/Inputs/InputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBase::InputBase(const Controller &controller, const MapIO io_adr, const Point &incoming_point)
    : LogicInputElement(controller, io_adr), ChainItem(incoming_point), LabeledLogicItem(name) {
    this->name = MapIONames[io_adr];
    this->io_adr = io_adr;
    this->prior_item = NULL;
}

InputBase::InputBase(const MapIO io_adr, InputBase &prior_item)
    : LogicInputElement(prior_item.controller, io_adr), ChainItem(prior_item.OutcomingPoint()),
      LabeledLogicItem(name) {
    this->io_adr = io_adr;
    this->prior_item = &prior_item;
}

InputBase::~InputBase() {
}

void InputBase::Render(uint8_t *fb) {
    auto bitmap = GetCurrentBitmap();

    LogicItemState prior_item_state = prior_item != NULL ? prior_item->state : state;
    if (prior_item_state == LogicItemState::lisActive) {
        draw_active_network(fb,
                            incoming_point.x,
                            incoming_point.y,
                            LabeledLogicItem::width + LeftPadding);
    } else {
        draw_passive_network(fb,
                             incoming_point.x,
                             incoming_point.y,
                             LabeledLogicItem::width + LeftPadding,
                             false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;
    draw_text_f6X12(fb, x_pos, incoming_point.y - LabeledLogicItem::height, label);

    x_pos += LabeledLogicItem::width;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    if (state == LogicItemState::lisActive) {
        draw_active_network(fb, x_pos, incoming_point.y, RightPadding);
    } else {
        draw_passive_network(fb, x_pos, incoming_point.y, RightPadding, true);
    }
}

Point InputBase::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}