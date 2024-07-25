#include "InputNO.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t inputNO_width = 16;
uint8_t inputNO_height = 16;
uint8_t inputNO_active[] = { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
                             0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 };

InputNO::InputNO(const MapIO io_adr, const Point &location)
    : LogicItemBase(), DisplayItemBase(location, { inputNO_width, inputNO_height }) {
}

InputNO::~InputNO() {
}

bool InputNO::DoAction() {
    return true;
}

uint8_t *InputNO::GetBitmap() {
    return inputNO_active;
}