#include "LogicProgram/Outputs/DirectOutput.h"
#include "Display/bitmaps/direct_output_active.h"
#include "Display/bitmaps/direct_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DirectOutput = "DirectOutput";

DirectOutput::DirectOutput() : CommonOutput() {
}

DirectOutput::DirectOutput(const MapIO io_adr) : DirectOutput() {
    SetIoAdr(io_adr);
}

DirectOutput::~DirectOutput() {
}

bool DirectOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    (void)prev_elem_changed;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    bool any_changes = false;
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        SetValue(state == LogicItemState::lisActive ? LogicElement::MaxValue
                                                    : LogicElement::MinValue);
        any_changes = true;
        ESP_LOGD(TAG_DirectOutput, ".");
    }

    return any_changes;
}

const Bitmap *DirectOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &DirectOutput::bitmap_active;

        default:
            return &DirectOutput::bitmap_passive;
    }
}

TvElementType DirectOutput::GetElementType() {
    return TvElementType::et_DirectOutput;
}