#include "LogicProgram/Outputs/DecOutput.h"
#include "Display/bitmaps/dec_output_active.h"
#include "Display/bitmaps/dec_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_DecOutput = "DecOutput";

DecOutput::DecOutput() : CommonOutput() {
}

DecOutput::DecOutput(const MapIO io_adr) : DecOutput() {
    SetIoAdr(io_adr);
}

DecOutput::~DecOutput() {
}

bool DecOutput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
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
            if (prev_val > LogicElement::MinValue) {
                prev_val--;
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

    ESP_LOGD(TAG_DecOutput, ". %u", Input->PeekValue());

    return true;
}

const Bitmap *DecOutput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
        case LogicItemState::lisStop:
            return &DecOutput::bitmap_active;

        default:
            return &DecOutput::bitmap_passive;
    }
}

TvElementType DecOutput::GetElementType() {
    return TvElementType::et_DecOutput;
}

DecOutput *DecOutput::TryToCast(CommonOutput *common_output) {
    switch (common_output->GetElementType()) {
        case TvElementType::et_DecOutput:
            return static_cast<DecOutput *>(common_output);

        default:
            return NULL;
    }
}

const AllowedIO DecOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}