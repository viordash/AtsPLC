#include "LogicProgram/Outputs/ResetOutput.h"
#include "Display/bitmaps/reset_output_active.h"
#include "Display/bitmaps/reset_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ResetOutput::ResetOutput(const MapIO io_adr, InputBase &prior_item)
    : OutputBase(io_adr, prior_item) {
}

ResetOutput::~ResetOutput() {
}

bool ResetOutput::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *ResetOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &ResetOutput::bitmap_active;

        default:
            return &ResetOutput::bitmap_passive;
    }
}
