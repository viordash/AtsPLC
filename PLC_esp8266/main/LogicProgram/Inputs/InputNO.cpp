#include "LogicProgram/Inputs/InputNO.h"
#include "Display/bitmaps/input_open_active.h"
#include "Display/bitmaps/input_open_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNO::InputNO(const MapIO io_adr, InputBase *incoming_item) : CommonInput(io_adr ,incoming_item) {
}

InputNO::~InputNO() {
}

bool InputNO::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *InputNO::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &InputNO::bitmap_active;

        default:
            return &InputNO::bitmap_passive;
    }
}