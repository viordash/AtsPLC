#include "LogicProgram/Outputs/ResetOutput.h"
#include "Display/bitmaps/reset_output_active.h"
#include "Display/bitmaps/reset_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ResetOutput::ResetOutput(const MapIO io_adr, InputBase &prev_item)
    : OutputBase(io_adr, prev_item) {
}

ResetOutput::~ResetOutput() {
}

bool ResetOutput::DoAction() {
    return true;
}

const Bitmap *ResetOutput::GetCurrentBitmap() {
    switch (incoming_item_state) {
        case LogicItemState::lisActive:
            return &ResetOutput::bitmap_active;

        default:
            return &ResetOutput::bitmap_passive;
    }
}
