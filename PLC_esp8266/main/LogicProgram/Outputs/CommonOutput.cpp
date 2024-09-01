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

bool CommonOutput::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    uint8_t total_widht = bitmap->size.width + LabeledLogicItem::width;
    uint8_t incoming_width = (OUTCOME_RAIL_LEFT - start_point->x) - total_widht;

    ESP_LOGD(TAG_CommonOutput,
             "x:%u, total_widht:%u, incoming_width:%u, OUTCOME_RAIL_LEFT:%u",
             start_point->x,
             total_widht,
             incoming_width,
             OUTCOME_RAIL_LEFT);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, incoming_width);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, incoming_width, false);
    }

    if (!res) {
        return res;
    }

    start_point->x += incoming_width;
    draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);

    start_point->x += bitmap->size.width;
    res = draw_text_f6X12(fb, start_point->x, start_point->y - LabeledLogicItem::height, label);
    if (!res) {
        return res;
    }

    if (state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, LabeledLogicItem::width);
    } else {
        res =
            draw_passive_network(fb, start_point->x, start_point->y, LabeledLogicItem::width, true);
    }
    return res;
}

size_t CommonOutput::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!WriteRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonOutput::Deserialize(uint8_t *buffer, size_t buffer_size) {
    (void)buffer;
    (void)buffer_size;
    size_t readed = 0;

    return readed;
}