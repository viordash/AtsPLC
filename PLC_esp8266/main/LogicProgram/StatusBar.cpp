#include "LogicProgram/StatusBar.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
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

IRAM_ATTR void StatusBar::Render(FrameBuffer *fb) {
    uint8_t separator_width = 1;
    Point point = { 2, y };

    indicator_AI->Render(fb, &point, Controller::AI.PeekValue());
    point.x += separator_width;

    indicator_DI->Render(fb, &point, Controller::DI.PeekValue());
    point.x += separator_width;

    indicator_O1->Render(fb, &point, Controller::O1.PeekValue());
    point.x += separator_width;

    indicator_O2->Render(fb, &point, Controller::O2.PeekValue());
    point.x += separator_width;

    indicator_V1->Render(fb, &point, Controller::V1.PeekValue());
    point.x += separator_width;

    indicator_V2->Render(fb, &point, Controller::V2.PeekValue());
    point.x += separator_width;

    indicator_V3->Render(fb, &point, Controller::V3.PeekValue());
    point.x += separator_width;

    separator_width = 0;
    indicator_V4->Render(fb, &point, Controller::V4.PeekValue());
    point.x += separator_width;

    ASSERT(draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH));
}