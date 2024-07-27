#include "LogicProgram/InputNO.h"
#include "Display/bitmaps/input_open_active.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNO::InputNO(const MapIO io_adr, const Point &location)
    : LogicItemBase(), LabeledLogicItem(MapIONames[io_adr], location) {
    this->io_adr = io_adr;
}

InputNO::~InputNO() {
}

bool InputNO::DoAction() {
    return true;
}

void InputNO::Render(uint8_t *fb) {
    LabeledLogicItem::Render(fb);

    uint8_t x_pos = incoming_point.x;
    uint8_t y_pos = incoming_point.y;
    draw_network(x_pos, y_pos, LabeledLogicItem::width);

    x_pos += LabeledLogicItem::width;
    y_pos -= (InputNO::bitmap.size.height / 2) - 1;
    DisplayItemBase::draw(fb, x_pos, y_pos, InputNO::bitmap);
}

Point InputNO::OutcomingPoint() {
    uint8_t x_pos = incoming_point.x + LabeledLogicItem::width + InputNO::bitmap.size.width;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}