#include "LogicProgram/Outputs/SetOutput.h"
#include "Display/bitmaps/set_output_active.h"
#include "Display/bitmaps/set_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SetOutput = "SetOutput";

SetOutput::SetOutput() : CommonOutput() {
}

SetOutput::SetOutput(const MapIO io_adr) : SetOutput() {
    SetIoAdr(io_adr);
}

SetOutput::~SetOutput() {
}

bool SetOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
            SetValue(LogicElement::MaxValue);
        }
        any_changes = true;
        ESP_LOGD(TAG_SetOutput, ".");
    }

    return any_changes;
}

const Bitmap *SetOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &SetOutput::bitmap_active;

        default:
            return &SetOutput::bitmap_passive;
    }
}

TvElementType SetOutput::GetElementType() {
    return TvElementType::et_SetOutput;
}