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
    if (!DoActionGuard(prev_elem_changed, prev_elem_state)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(lock_mutex);

    if (prev_elem_state == state) {
        return false;
    }

    state = prev_elem_state;

    switch (state) {
        case LogicItemState::lisActive:
            Output->WriteValue(LogicElement::MaxValue);
            break;

        case LogicItemState::lisPassive:
            break;

        case LogicItemState::lisStop:
            Output->WriteValue(LogicElement::MinValue);
            break;
    }

    ESP_LOGD(TAG_SetOutput, ".");

    return true;
}

const Bitmap *SetOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
        case LogicItemState::lisStop:
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