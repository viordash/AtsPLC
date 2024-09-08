#include "LogicProgram/Outputs/IncOutput.h"
#include "Display/bitmaps/inc_output_active.h"
#include "Display/bitmaps/inc_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_IncOutput = "IncOutput";

IncOutput::IncOutput() : CommonOutput() {
}

IncOutput::IncOutput(const MapIO io_adr) : IncOutput() {
    SetIoAdr(io_adr);
}

IncOutput::~IncOutput() {
}

bool IncOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        if (state == LogicItemState::lisActive) {
            uint8_t prev_val = GetValue();
            if (prev_val > LogicElement::MinValue) {
                prev_val++;
            }
            SetValue(prev_val);
        }
        any_changes = true;
        ESP_LOGD(TAG_IncOutput, ".");
    }

    return any_changes;
}

const Bitmap *IncOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &IncOutput::bitmap_active;

        default:
            return &IncOutput::bitmap_passive;
    }
}

TvElementType IncOutput::GetElementType() {
    return TvElementType::et_IncOutput;
}