#include "Display/DisplayItemBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DisplayItemBase::DisplayItemBase(const Point &location) {
    this->location = location;
}

DisplayItemBase::~DisplayItemBase() {
}

const Point &DisplayItemBase::GetLocation() {
    return location;
}

void DisplayItemBase::draw(uint8_t *fb, int8_t x, int8_t y, const Bitmap &bitmap) {
    for (int row = y; row < y + bitmap.size.height; row += 8) {
        if (row >= DISPLAY_HEIGHT) {
            continue;
        }
        for (int column = x; column < x + bitmap.size.width; column++) {
            if (column >= DISPLAY_WIDTH) {
                continue;
            }
            int src_id = (((row - y) / 8) * bitmap.size.width) + (column - x);
            int dst_id = ((row / 8) * DISPLAY_WIDTH) + column;

            uint8_t b = bitmap.data[src_id];
            fb[dst_id] |= b << (y % 8);
            if ((y % 8) > 0 && row + 1 < DISPLAY_HEIGHT) {
                fb[dst_id + DISPLAY_WIDTH] |= b >> (8 - (y % 8));
            }
        }
    }
}