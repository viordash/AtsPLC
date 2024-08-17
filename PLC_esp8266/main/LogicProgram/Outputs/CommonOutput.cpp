#include "LogicProgram/Outputs/CommonOutput.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonOutput = "CommonOutput";

CommonOutput::CommonOutput(const MapIO io_adr, InputBase *incoming_item)
    : StatefulElement(incoming_item->controller), LogicOutputElement(io_adr),
      DisplayChainItem(incoming_item->OutcomingPoint()), LabeledLogicItem(MapIONames[io_adr]) {
    this->incoming_item = incoming_item;
    this->incoming_item->Bind(this);
}

CommonOutput::~CommonOutput() {
}

bool CommonOutput::Render(uint8_t *fb) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    uint8_t total_widht = bitmap->size.width + LabeledLogicItem::width + RightPadding;
    uint8_t incoming_width = (OUTCOME_RAIL_LEFT - incoming_point.x) - total_widht;

    ESP_LOGD(TAG_CommonOutput,
             "x:%u, total_widht:%u, incoming_width:%u, OUTCOME_RAIL_LEFT:%u",
             incoming_point.x,
             total_widht,
             incoming_width,
             OUTCOME_RAIL_LEFT);

    if (incoming_item->GetState() == LogicItemState::lisActive) {
        res &= draw_active_network(fb, incoming_point.x, incoming_point.y, incoming_width);
    } else {
        res &= draw_passive_network(fb, incoming_point.x, incoming_point.y, incoming_width, false);
    }

    uint8_t x_pos = incoming_point.x + incoming_width;
    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    res &= draw_text_f6X12(fb, x_pos, incoming_point.y - LabeledLogicItem::height, label);

    if (incoming_item->GetState() == LogicItemState::lisActive) {
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
    uint8_t x_pos = OUTCOME_RAIL_LEFT;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}