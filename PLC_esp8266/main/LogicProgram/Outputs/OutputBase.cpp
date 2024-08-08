#include "LogicProgram/Outputs/OutputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutputBase::OutputBase(const MapIO io_adr, InputBase &prior_item)
    : LogicOutputElement(prior_item.controller, io_adr), NetworkedLogicItem(),
      LabeledLogicItem(name) {
    this->io_adr = io_adr;
    this->prior_item = &prior_item;
    this->incoming_point = prior_item.OutcomingPoint();
}

OutputBase::~OutputBase() {
}

void OutputBase::Render(uint8_t *fb) {
    auto bitmap = GetCurrentBitmap();

    LogicItemState prior_item_state =
        prior_item != NULL ? prior_item->state : LogicItemState::lisPassive;
    if (prior_item_state == LogicItemState::lisActive) {
        draw_active_network(fb, incoming_point.x, incoming_point.y, LeftPadding);
    } else {
        draw_passive_network(fb, incoming_point.x, incoming_point.y, LeftPadding, false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    draw_text_f6X12(fb, x_pos, incoming_point.y - LabeledLogicItem::height, label);

    if (prior_item_state == LogicItemState::lisActive) {
        draw_active_network(fb, x_pos, incoming_point.y, LabeledLogicItem::width + RightPadding);
    } else {
        draw_passive_network(fb,
                             x_pos,
                             incoming_point.y,
                             LabeledLogicItem::width + RightPadding,
                             true);
    }
}

Point OutputBase::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}