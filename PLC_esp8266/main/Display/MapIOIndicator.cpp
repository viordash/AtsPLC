#include "Display/MapIOIndicator.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MapIOIndicator::MapIOIndicator(const MapIO io_adr) {
    this->name = MapIONames[io_adr];
}

MapIOIndicator::~MapIOIndicator() {
}

IRAM_ATTR bool MapIOIndicator::Render(uint8_t *fb, Point *start_point, uint8_t progress) {
    bool res;

    start_point->x += margin;

    res = draw_horz_progress_bar(fb, start_point->x, start_point->y, progress);

    res &= draw_text_f5X7(fb,
                          start_point->x + margin,
                          start_point->y + margin + HORZ_PROGRESS_BAR_HEIGHT,
                          name);

    start_point->x += (text_width * name_size) + margin + margin;
    return res;
}

uint8_t MapIOIndicator::GetHeight() {
    return HORZ_PROGRESS_BAR_HEIGHT + text_height;
}