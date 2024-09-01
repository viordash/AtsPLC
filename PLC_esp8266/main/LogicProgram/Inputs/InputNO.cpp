#include "LogicProgram/Inputs/InputNO.h"
#include "Display/bitmaps/input_open_active.h"
#include "Display/bitmaps/input_open_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNO = "InputNO";

InputNO::InputNO(const MapIO io_adr) : CommonInput(io_adr) {
}

InputNO::~InputNO() {
}

bool InputNO::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_changed;
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
        && GetValue() != LogicElement::MinValue) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_InputNO, ".");
    }

    return any_changes;
}

const Bitmap *InputNO::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &InputNO::bitmap_active;

        default:
            return &InputNO::bitmap_passive;
    }
}

size_t InputNO::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = et_InputNO;

    if (!WriteRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!WriteRecord(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }

    return writed;
}

size_t InputNO::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    TvElement tvElement;

    if (!ReadRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (tvElement.type != et_InputNO) {
        return 0;
    }

    if (!ReadRecord(&io_adr, sizeof(io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }

    return readed;
}