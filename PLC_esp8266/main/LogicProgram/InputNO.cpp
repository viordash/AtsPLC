#include "LogicProgram/InputNO.h"
#include "LogicProgram/InputNOBitmap.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNO::InputNO(const MapIO io_adr, const Point &location)
    : LogicItemBase(),
      DisplayItemBase(location, { InputNO::bitmap.size.width, InputNO::bitmap.size.height }) {
    this->io_adr = io_adr;
}

InputNO::~InputNO() {
}

bool InputNO::DoAction() {
    return true;
}

const uint8_t *InputNO::GetBitmap() {
    return InputNO::bitmap.data;
}