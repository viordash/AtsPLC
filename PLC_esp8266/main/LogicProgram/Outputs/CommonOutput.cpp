#include "LogicProgram/Outputs/CommonOutput.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonOutput = "CommonOutput";

CommonOutput::CommonOutput(const MapIO io_adr)
    : LogicElement(), InputOutputElement(io_adr), LabeledLogicItem(MapIONames[io_adr]) {
}

CommonOutput::~CommonOutput() {
}

bool CommonOutput::Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    uint8_t total_widht = bitmap->size.width + LabeledLogicItem::width + RightPadding;
    uint8_t incoming_width = (OUTCOME_RAIL_LEFT - start_point.x) - total_widht;

    ESP_LOGD(TAG_CommonOutput,
             "x:%u, total_widht:%u, incoming_width:%u, OUTCOME_RAIL_LEFT:%u",
             start_point.x,
             total_widht,
             incoming_width,
             OUTCOME_RAIL_LEFT);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point.x, start_point.y, incoming_width);
    } else {
        res = draw_passive_network(fb, start_point.x, start_point.y, incoming_width, false);
    }

    if (!res) {
        return res;
    }

    uint8_t x_pos = start_point.x + incoming_width;
    draw_bitmap(fb, x_pos, start_point.y - (bitmap->size.height / 2) + 1, bitmap);

    x_pos += bitmap->size.width;
    res = draw_text_f6X12(fb, x_pos, start_point.y - LabeledLogicItem::height, label);
    if (!res) {
        return res;
    }

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, x_pos, start_point.y, LabeledLogicItem::width + RightPadding);
    } else {
        res = draw_passive_network(fb,
                                   x_pos,
                                   start_point.y,
                                   LabeledLogicItem::width + RightPadding,
                                   true);
    }
    return res;
}

Point CommonOutput::OutcomingPoint() {
    // uint8_t x_pos = OUTCOME_RAIL_LEFT;
    // uint8_t y_pos = incoming_point.y;
    // return { x_pos, y_pos };
    return { 0, 0 };
}