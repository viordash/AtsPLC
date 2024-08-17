#include "LogicProgram/Inputs/InputNO.h"
#include "Display/bitmaps/input_open_active.h"
#include "Display/bitmaps/input_open_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNO = "InputNO";

InputNO::InputNO(const MapIO io_adr, InputBase *incoming_item)
    : CommonInput(io_adr, incoming_item) {
}

InputNO::~InputNO() {
}

bool InputNO::DoAction() {
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (incoming_item->GetState() == LogicItemState::lisActive //
        && GetValue() != StatefulElement::MinValue) {
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

bool InputNO::Render(uint8_t *fb) {
    bool res = CommonInput::Render(fb);
    return res;
}