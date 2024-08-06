#include "LogicProgram/Outputs/DecOutput.h"
#include "Display/bitmaps/dec_output_active.h"
#include "Display/bitmaps/dec_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DecOutput::DecOutput(const MapIO io_adr, InputBase &prior_item)
    : OutputBase(io_adr, prior_item) {
}

DecOutput::~DecOutput() {
}

bool DecOutput::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *DecOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &DecOutput::bitmap_active;

        default:
            return &DecOutput::bitmap_passive;
    }
}
