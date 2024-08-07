#include "Display/StatusBar.h"
#include "Display/Common.h"
#include "Display/MapIOIndicator.h"
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
    Point point = { 0, y };
    MapIOIndicator indicator_AI(point, MapIONames[MapIO::AI], GetAIRelativeValue());
    indicator_AI.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_DI(point, MapIONames[MapIO::DI], GetDIRelativeValue());
    indicator_DI.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_O1(point, MapIONames[MapIO::O1], GetO1RelativeValue());
    indicator_O1.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_O2(point, MapIONames[MapIO::O2], GetO2RelativeValue());
    indicator_O2.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_V1(point, MapIONames[MapIO::V1], GetV1RelativeValue());
    indicator_V1.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_V2(point, MapIONames[MapIO::V2], GetV2RelativeValue());
    indicator_V2.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_V3(point, MapIONames[MapIO::V3], GetV3RelativeValue());
    indicator_V3.Render(fb);

    point.x += MapIOIndicator::GetWidth();
    MapIOIndicator indicator_V4(point, MapIONames[MapIO::V4], GetV4RelativeValue());
    indicator_V4.Render(fb);

    draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH);
}