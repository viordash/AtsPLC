#include "LogicProgram/Outputs/IncOutput.h"
#include "Display/bitmaps/inc_output_active.h"
#include "Display/bitmaps/inc_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IncOutput::IncOutput(const MapIO io_adr, InputBase &prior_item)
    : OutputBase(io_adr, prior_item) {
}

IncOutput::~IncOutput() {
}

bool IncOutput::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *IncOutput::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &IncOutput::bitmap_active;

        default:
            return &IncOutput::bitmap_passive;
    }
}
