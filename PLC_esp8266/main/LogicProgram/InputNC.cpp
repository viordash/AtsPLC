#include "LogicProgram/InputNC.h"
#include "Display/bitmaps/input_close_active.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNC::InputNC(const MapIO io_adr, const Point &incoming_point)
    : LogicItemBase(), NetworkedLogicItem(incoming_point), LabeledLogicItem(MapIONames[io_adr]) {
    this->io_adr = io_adr;
}

InputNC::~InputNC() {
}

bool InputNC::DoAction() {
    return true;
}

void InputNC::SetOrigin() {
    origin = { (uint8_t)(incoming_point.x - LeftPadding), incoming_point.y };
}

void InputNC::Render(uint8_t *fb) {
    uint8_t x_pos = incoming_point.x;
    draw_network(x_pos, incoming_point.y, LabeledLogicItem::width + LeftPadding);

    x_pos += LeftPadding;
    draw_text_f6X12(x_pos, incoming_point.y - LabeledLogicItem::height, label);

    x_pos += LabeledLogicItem::width;
    draw_bitmap(fb,
                x_pos,
                incoming_point.y - (InputNC::bitmap_active.size.height / 2) + 1,
                &InputNC::bitmap_active);

    x_pos += InputNC::bitmap_active.size.width;
    draw_network(x_pos, incoming_point.y, RightPadding);
}

Point InputNC::OutcomingPoint() {
    uint8_t x_pos = LeftPadding + incoming_point.x + LabeledLogicItem::width
                  + InputNC::bitmap_active.size.width + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}