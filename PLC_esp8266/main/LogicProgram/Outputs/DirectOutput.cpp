#include "LogicProgram/Outputs/DirectOutput.h"
#include "Display/bitmaps/direct_output_active.h"
#include "Display/bitmaps/direct_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DirectOutput::DirectOutput(const MapIO io_adr, InputBase &prior_item) : OutputBase(io_adr, prior_item) {
}

DirectOutput::~DirectOutput() {
}

bool DirectOutput::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *DirectOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &DirectOutput::bitmap_active;

        default:
            return &DirectOutput::bitmap_passive;
    }
}
