#include "LogicProgram/Outputs/ResetOutput.h"
#include "Display/bitmaps/reset_output_active.h"
#include "Display/bitmaps/reset_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ResetOutput = "ResetOutput";

ResetOutput::ResetOutput() : CommonOutput() {
}

ResetOutput::ResetOutput(const MapIO io_adr) : ResetOutput() {
    SetIoAdr(io_adr);
}

ResetOutput::~ResetOutput() {
}

bool ResetOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        if (state == LogicItemState::lisActive) {
            Output->WriteValue(LogicElement::MinValue);
        }
        any_changes = true;
        ESP_LOGD(TAG_ResetOutput, ".");
    }

    return any_changes;
}

const Bitmap *ResetOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &ResetOutput::bitmap_active;

        default:
            return &ResetOutput::bitmap_passive;
    }
}

TvElementType ResetOutput::GetElementType() {
    return TvElementType::et_ResetOutput;
}

ResetOutput *ResetOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_ResetOutput:
            return static_cast<ResetOutput *>(common_output);

        default:
            return NULL;
    }
}

const AllowedIO ResetOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::O1, MapIO::O2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}