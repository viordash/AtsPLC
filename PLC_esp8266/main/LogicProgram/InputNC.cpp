#include "LogicProgram/InputNC.h"
#include "Display/bitmaps/input_close_active.h"
#include "Display/bitmaps/input_close_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNC::InputNC(const MapIO io_adr, const Point &incoming_point)
    : InputBase(io_adr, incoming_point) {
    this->io_adr = io_adr;
}

InputNC::~InputNC() {
}

bool InputNC::DoAction() {
    return true;
}

const Bitmap *InputNC::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &InputNC::bitmap_active;

        default:
            return &InputNC::bitmap_passive;
    }
}
