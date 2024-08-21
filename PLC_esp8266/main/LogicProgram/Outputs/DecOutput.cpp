#include "LogicProgram/Outputs/DecOutput.h"
#include "Display/bitmaps/dec_output_active.h"
#include "Display/bitmaps/dec_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DecOutput = "DecOutput";

DecOutput::DecOutput(const MapIO io_adr, InputBase *incoming_item)
    : CommonOutput(io_adr, incoming_item) {
}

DecOutput::~DecOutput() {
}

bool DecOutput::DoAction(bool prev_changed) {
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
            uint8_t prev_val = GetValue();
            if (prev_val > LogicElement::MinValue) {
                prev_val--;
            }
            SetValue(prev_val);
        }
        any_changes = true;
        ESP_LOGD(TAG_DecOutput, ".");
    }

    return any_changes;
}

const Bitmap *DecOutput::GetCurrentBitmap() {
    switch (incoming_item->GetState()) {
        case LogicItemState::lisActive:
            return &DecOutput::bitmap_active;

        default:
            return &DecOutput::bitmap_passive;
    }
}
