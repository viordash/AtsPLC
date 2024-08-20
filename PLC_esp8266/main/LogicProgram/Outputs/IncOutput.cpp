#include "LogicProgram/Outputs/IncOutput.h"
#include "Display/bitmaps/inc_output_active.h"
#include "Display/bitmaps/inc_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IncOutput::IncOutput(const MapIO io_adr, InputBase *incoming_item)
    : CommonOutput(io_adr ,incoming_item) {
}

IncOutput::~IncOutput() {
}

bool IncOutput::DoAction(bool prev_changed) {
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
            if (prev_val > StatefulElement::MinValue) {
                prev_val++;
            }
            SetValue(prev_val);
        }
        any_changes = true;
        ESP_LOGD(TAG_DecOutput, ".");
    }

    return any_changes;
}

const Bitmap *IncOutput::GetCurrentBitmap() {
    switch (incoming_item->GetState()) {
        case LogicItemState::lisActive:
            return &IncOutput::bitmap_active;

        default:
            return &IncOutput::bitmap_passive;
    }
}
