#include "LogicProgram/Outputs/DecOutput.h"
#include "Display/bitmaps/dec_output_active.h"
#include "Display/bitmaps/dec_output_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DecOutput::DecOutput(const MapIO io_adr, InputBase *incoming_item)
    : CommonOutput(io_adr ,incoming_item) {
}

DecOutput::~DecOutput() {
}

bool DecOutput::DoAction() {

    return true;
}

const Bitmap *DecOutput::GetCurrentBitmap() {
    switch (incoming_item->GetState()) {
        case LogicItemState::lisActive:
            return &DecOutput::bitmap_active;

        default:
            return &DecOutput::bitmap_passive;
    }
}
