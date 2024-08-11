#include "LogicProgram/Outputs/SetOutput.h"
#include "Display/bitmaps/set_output_active.h"
#include "Display/bitmaps/set_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SetOutput::SetOutput(const MapIO io_adr, InputBase *incoming_item) : CommonOutput(io_adr ,incoming_item) {
}

SetOutput::~SetOutput() {
}

bool SetOutput::DoAction() {
    return true;
}

const Bitmap *SetOutput::GetCurrentBitmap() {
    switch (incoming_item_state) {
        case LogicItemState::lisActive:
            return &SetOutput::bitmap_active;

        default:
            return &SetOutput::bitmap_passive;
    }
}
