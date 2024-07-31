#include "LogicProgram/InputRail.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputRail::InputRail(uint8_t network_number)
    : InputBase(MapIO::Undef,
                { 0, (uint8_t)(INPUT_RAIL_TOP + INPUT_RAIL_HEIGHT * network_number) }) {
    this->network_number = network_number;
    state = LogicItemState::lisActive;
}

InputRail::~InputRail() {
}

bool InputRail::DoAction() {
    return true;
}

const Bitmap *InputRail::GetCurrentBitmap() {
    return NULL;
}

void InputRail::Render(uint8_t *fb) {
    draw_input_rail(network_number);
}

Point InputRail::OutcomingPoint() {
    uint8_t x_pos = 2;
    uint8_t y_pos = incoming_point.y + (INPUT_RAIL_HEIGHT / 2);
    return { x_pos, y_pos };
}