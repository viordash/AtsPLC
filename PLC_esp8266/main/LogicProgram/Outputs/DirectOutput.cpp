#include "LogicProgram/Outputs/DirectOutput.h"
#include "Display/bitmaps/direct_output_active.h"
#include "Display/bitmaps/direct_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DirectOutput::DirectOutput(const MapIO io_adr, InputBase &prev_item) : OutputBase(io_adr, prev_item) {
}

DirectOutput::~DirectOutput() {
}

bool DirectOutput::DoAction() {
    return true;
}

const Bitmap *DirectOutput::GetCurrentBitmap() {
    switch (prev_item->state) {
        case LogicItemState::lisActive:
            return &DirectOutput::bitmap_active;

        default:
            return &DirectOutput::bitmap_passive;
    }
}
