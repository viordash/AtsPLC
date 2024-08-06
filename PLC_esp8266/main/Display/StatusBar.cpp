#include "Display/StatusBar.h"
#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatusBar::StatusBar(uint8_t y) : DisplayItemBase() {
    this->y = y;
}

StatusBar::~StatusBar() {
}

void StatusBar::SetOrigin() {
    origin = { 0, y };
}

static uint8_t GetAIRelativeValue() {
    return 10;
}
static uint8_t GetDIRelativeValue() {
    return 100;
}
static uint8_t GetO1RelativeValue() {
    return 100;
}
static uint8_t GetO2RelativeValue() {
    return 100;
}
static uint8_t GetV1RelativeValue() {
    return 25;
}
static uint8_t GetV2RelativeValue() {
    return 50;
}
static uint8_t GetV3RelativeValue() {
    return 75;
}
static uint8_t GetV4RelativeValue() {
    return 0;
}

void StatusBar::Render(uint8_t *fb) {

    uint8_t x_pos = 0;
    const uint8_t text_width = 6;
    const uint8_t mapIO_name_size = 2;
    const uint8_t text_height = 12;
    const uint8_t component_height = text_height + 2;

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::AI]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetAIRelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::DI]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetDIRelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::O1]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetO1RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::O2]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetO2RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::V1]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetV1RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::V2]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetV2RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::V3]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetV3RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_text_f6X12(fb, x_pos, y - text_height, MapIONames[MapIO::V4]);
    x_pos += text_width * mapIO_name_size;
    draw_progress_bar(fb, x_pos, y, GetV4RelativeValue());
    x_pos += PROGRESS_BAR_WIDTH;
    draw_vert_line(fb, x_pos, y, component_height);

    draw_horz_line(fb, 0, y + component_height, DISPLAY_WIDTH);
}