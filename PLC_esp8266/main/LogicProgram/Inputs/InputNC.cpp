#include "LogicProgram/Inputs/InputNC.h"
#include "Display/bitmaps/input_close_active.h"
#include "Display/bitmaps/input_close_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNC = "InputNC";

InputNC::InputNC(const MapIO io_adr) : CommonInput(io_adr) {
}

InputNC::~InputNC() {
}

bool InputNC::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_changed;
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
        && GetValue() == LogicElement::MinValue) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_InputNC, ".");
    }
    return any_changes;
}

const Bitmap *InputNC::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &InputNC::bitmap_active;

        default:
            return &InputNC::bitmap_passive;
    }
}

size_t InputNC::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = et_InputNC;

    if (!WriteRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!WriteRecord(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }

    return writed;
}

size_t InputNC::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    TvElement tvElement;

    if (!ReadRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (tvElement.type != et_InputNC) {
        return 0;
    }

    if (!ReadRecord(&io_adr, sizeof(io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }

    return readed;
}