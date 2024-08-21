#include "LogicProgram/Inputs/InputNC.h"
#include "Display/bitmaps/input_close_active.h"
#include "Display/bitmaps/input_close_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_InputNC = "InputNC";

InputNC::InputNC(const MapIO io_adr, InputBase *incoming_item)
    : CommonInput(io_adr, incoming_item) {
}

InputNC::~InputNC() {
}

bool InputNC::DoAction(bool prev_changed) {
    (void)prev_changed;
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (incoming_item->GetState() == LogicItemState::lisActive //
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
