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
    DisplayItemBase::draw(fb, location.x + 8 * 2, location.y + 4, InputNO::bitmap);
    draw_IO_name(location.x, location.y, label);
}