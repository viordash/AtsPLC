#include "LogicProgram/Inputs/InputNC.h"
#include "Display/bitmaps/input_close_active.h"
#include "Display/bitmaps/input_close_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNC::InputNC(const MapIO io_adr, InputBase *incoming_item)
    : CommonInput(io_adr, incoming_item) {
}

InputNC::~InputNC() {
}

bool InputNC::DoAction() {
    bool any_changes = false;
    LogicItemState prev_state = state;

    if (incoming_item->GetState() == LogicItemState::lisActive //
        && GetValue() == 0) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        require_render = true;
        any_changes = true;
    }
    return any_changes;
}

const Bitmap *InputNC::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &InputNC::bitmap_active;

        default:
            return &InputNC::bitmap_passive;
    }
}

bool InputNC::Render(uint8_t *fb) {
    if (!require_render) {
        return true;
    }
    bool res = CommonInput::Render(fb);
    require_render = false;
    return res;
}
