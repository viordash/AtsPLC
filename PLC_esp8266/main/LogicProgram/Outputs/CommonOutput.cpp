#include "LogicProgram/Outputs/CommonOutput.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommonOutput::CommonOutput(const MapIO io_adr, InputBase &prev_item)
    : LogicElement(prev_item.controller), LogicOutputElement(io_adr), DisplayChainItem(prev_item),
      LabeledLogicItem(MapIONames[io_adr]) {
}

CommonOutput::~CommonOutput() {
}

bool CommonOutput::Render(uint8_t *fb) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    if (incoming_item_state == LogicItemState::lisActive) {
        res &= draw_active_network(fb, incoming_point.x, incoming_point.y, LeftPadding);
    } else {
        res &= draw_passive_network(fb, incoming_point.x, incoming_point.y, LeftPadding, false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    res &= draw_text_f6X12(fb, x_pos, incoming_point.y - LabeledLogicItem::height, label);

    if (incoming_item_state == LogicItemState::lisActive) {
        res &= draw_active_network(fb,
                                   x_pos,
                                   incoming_point.y,
                                   LabeledLogicItem::width + RightPadding);
    } else {
        res &= draw_passive_network(fb,
                                    x_pos,
                                    incoming_point.y,
                                    LabeledLogicItem::width + RightPadding,
                                    true);
    }
    return res;
}

Point CommonOutput::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width + bitmap->size.width
                  + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}