#include "LogicProgram/Outputs/SetOutput.h"
#include "Display/bitmaps/set_output_active.h"
#include "Display/bitmaps/set_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SetOutput = "SetOutput";

SetOutput::SetOutput(const MapIO io_adr, InputBase *incoming_item)
    : CommonOutput(io_adr, incoming_item) {
}

SetOutput::~SetOutput() {
}

bool SetOutput::DoAction(bool prev_changed) {
    (void)prev_changed;
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (incoming_item->GetState() == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        if (state == LogicItemState::lisActive) {
            SetValue(LogicElement::MaxValue);
        }
        any_changes = true;
        ESP_LOGD(TAG_SetOutput, ".");
    }

    return any_changes;
}

const Bitmap *SetOutput::GetCurrentBitmap() {
    switch (incoming_item->GetState()) {
        case LogicItemState::lisActive:
            return &SetOutput::bitmap_active;

        default:
            return &SetOutput::bitmap_passive;
    }
}
