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
    if (!DoActionGuard(prev_elem_changed, prev_elem_state)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(lock_mutex);

    if (prev_elem_state == state) {
        return false;
    }

    state = prev_elem_state;

    switch (state) {
        case LogicItemState::lisActive: {
            uint8_t prev_val = Input->PeekValue();
            if (prev_val < LogicElement::MaxValue) {
                prev_val++;
            }
            Output->WriteValue(prev_val);
            break;
        }

        case LogicItemState::lisPassive:
            break;

        case LogicItemState::lisStop:
            Output->WriteValue(LogicElement::MinValue);
            break;
    }

    ESP_LOGD(TAG_IncOutput, ". %u", Input->PeekValue());

    return true;
}

const Bitmap *IncOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
        case LogicItemState::lisStop:
            return &IncOutput::bitmap_active;

        default:
            return &IncOutput::bitmap_passive;
    }
}

TvElementType IncOutput::GetElementType() {
    return TvElementType::et_IncOutput;
}

IncOutput *IncOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_IncOutput:
            return static_cast<IncOutput *>(common_output);

        default:
            return NULL;
    }
}

const AllowedIO IncOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}