#include "LogicProgram/StatusBar.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatusBar::StatusBar(uint8_t y) {
    this->y = y;

    indicator_AI = new MapIOIndicator(MapIO::AI);
    indicator_DI = new MapIOIndicator(MapIO::DI);
    indicator_O1 = new MapIOIndicator(MapIO::O1);
    indicator_O2 = new MapIOIndicator(MapIO::O2);
    indicator_V1 = new MapIOIndicator(MapIO::V1);
    indicator_V2 = new MapIOIndicator(MapIO::V2);
    indicator_V3 = new MapIOIndicator(MapIO::V3);
    indicator_V4 = new MapIOIndicator(MapIO::V4);
}

StatusBar::~StatusBar() {
    delete indicator_AI;
    delete indicator_DI;
    delete indicator_O1;
    delete indicator_O2;
    delete indicator_V1;
    delete indicator_V2;
    delete indicator_V3;
    delete indicator_V4;
}

bool StatusBar::Render(uint8_t *fb) {
    bool res = true;
    uint8_t separator_width = 1;
    Point point = { 2, y };

    res &= indicator_AI->Render(fb, &point, Controller::GetAIRelativeValue());
    point.x += separator_width;

    res &= indicator_DI->Render(fb, &point, Controller::GetDIRelativeValue());
    point.x += separator_width;

    res &= indicator_O1->Render(fb, &point, Controller::GetO1RelativeValue());
    point.x += separator_width;

    res &= indicator_O2->Render(fb, &point, Controller::GetO2RelativeValue());
    point.x += separator_width;

    res &= indicator_V1->Render(fb, &point, Controller::GetV1RelativeValue());
    point.x += separator_width;

    res &= indicator_V2->Render(fb, &point, Controller::GetV2RelativeValue());
    point.x += separator_width;

    res &= indicator_V3->Render(fb, &point, Controller::GetV3RelativeValue());
    point.x += separator_width;

    separator_width = 0;
    res &= indicator_V4->Render(fb, &point, Controller::GetV4RelativeValue());
    point.x += separator_width;

    res &= draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH);
    return res;
}