#include "LogicProgram/Outputs/OutputBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutputBase::OutputBase(const MapIO io_adr, const Point &incoming_point)
    : LogicItemBase(), NetworkedLogicItem(incoming_point), LabeledLogicItem(MapIONames[io_adr]) {
    this->io_adr = io_adr;
    this->prior_item = NULL;
}

OutputBase::OutputBase(const MapIO io_adr, InputBase &prior_item)
    : LogicItemBase(), NetworkedLogicItem(), LabeledLogicItem(MapIONames[io_adr]) {
    this->io_adr = io_adr;
    this->prior_item = &prior_item;
    this->incoming_point = prior_item.OutcomingPoint();
}

OutputBase::~OutputBase() {
}

void OutputBase::SetOrigin() {
    origin = { (uint8_t)(incoming_point.x - LeftPadding), incoming_point.y };
}

void OutputBase::Render(uint8_t *fb) {
    auto bitmap = GetCurrentBitmap();

    LogicItemState prior_item_state = prior_item != NULL ? prior_item->state : state;
    if (prior_item_state == LogicItemState::lisActive) {
        draw_active_network(incoming_point.x,
                            incoming_point.y,
                            LabeledLogicItem::width + LeftPadding);
    } else {
        draw_passive_network(incoming_point.x,
                             incoming_point.y,
                             LabeledLogicItem::width + LeftPadding,
                             false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;
    draw_text_f6X12(x_pos, incoming_point.y - LabeledLogicItem::height, label);

    x_pos += LabeledLogicItem::width;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    if (state == LogicItemState::lisActive) {
        draw_active_network(x_pos, incoming_point.y, RightPadding);
    } else {
        draw_passive_network(x_pos, incoming_point.y, RightPadding, true);
    }
}

Point OutputBase::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}