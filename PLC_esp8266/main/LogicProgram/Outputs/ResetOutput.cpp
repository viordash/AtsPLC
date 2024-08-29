#include "LogicProgram/Outputs/ResetOutput.h"
#include "Display/bitmaps/reset_output_active.h"
#include "Display/bitmaps/reset_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ResetOutput = "ResetOutput";

ResetOutput::ResetOutput(const MapIO io_adr, const Controller *controller)
    : CommonOutput(io_adr, controller) {
}

ResetOutput::~ResetOutput() {
}

bool ResetOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_changed;
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        if (state == LogicItemState::lisActive) {
            SetValue(LogicElement::MinValue);
        }
        any_changes = true;
        ESP_LOGD(TAG_ResetOutput, ".");
    }

    return any_changes;
}

const Bitmap *ResetOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ResetOutput::bitmap_active;

        default:
            return &ResetOutput::bitmap_passive;
    }
}
