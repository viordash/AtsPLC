#include "Display/MapIOIndicator.h"
#include "Display/display.h"
#include "LogicProgram/LogicElement.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MapIOIndicator::MapIOIndicator(const MapIO io_adr) {
    this->name = MapIONames[io_adr];
    progress = LogicElement::MinValue;
}

MapIOIndicator::~MapIOIndicator() {
}

IRAM_ATTR void MapIOIndicator::Render(FrameBuffer *fb, Point *start_point, uint8_t progress) {
    start_point->x += margin;

    ASSERT(draw_horz_progress_bar(fb, start_point->x, start_point->y, progress));

    ASSERT(draw_text_f5X7(fb,
                          start_point->x + margin,
                          start_point->y + margin + HORZ_PROGRESS_BAR_HEIGHT,
                          name)
           > 0);

    start_point->x += (text_width * name_size) + margin + margin;

    fb->has_changes |= this->progress != progress;
    this->progress = progress;
}

uint8_t MapIOIndicator::GetHeight() {
    return HORZ_PROGRESS_BAR_HEIGHT + text_height;
}