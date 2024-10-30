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
            SetValue(LogicElement::MaxValue);
        }
        any_changes = true;
        ESP_LOGD(TAG_SetOutput, ".");
    }

    return any_changes;
}

const Bitmap *SetOutput::GetCurrentBitmap(LogicItemState state) {
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

SetOutput *SetOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_SetOutput:
            return static_cast<SetOutput *>(common_output);

        default:
            return NULL;
    }
}

const AllowedIO SetOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::O1, MapIO::O2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}