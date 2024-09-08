#include "LogicProgram/Outputs/CommonOutput.h"
#include "LogicProgram/Serializer/Record.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonOutput = "CommonOutput";

CommonOutput::CommonOutput() : LogicElement(), InputOutputElement() {
}

CommonOutput::~CommonOutput() {
}

void CommonOutput::SetIoAdr(const MapIO io_adr) {
    InputOutputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
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
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonOutput::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    io_adr = _io_adr;
    return readed;
}