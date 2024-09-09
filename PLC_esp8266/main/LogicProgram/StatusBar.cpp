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
    MapIOIndicator indicator_AI(MapIONames[MapIO::AI],
                                Controller::GetAIRelativeValue(),
                                separator_width);
    res &= indicator_AI.Render(fb, &point);

    MapIOIndicator indicator_DI(MapIONames[MapIO::DI],
                                Controller::GetDIRelativeValue(),
                                separator_width);
    res &= indicator_DI.Render(fb, &point);

    MapIOIndicator indicator_O1(MapIONames[MapIO::O1],
                                Controller::GetO1RelativeValue(),
                                separator_width);
    res &= indicator_O1.Render(fb, &point);

    MapIOIndicator indicator_O2(MapIONames[MapIO::O2],
                                Controller::GetO2RelativeValue(),
                                separator_width);
    res &= indicator_O2.Render(fb, &point);

    MapIOIndicator indicator_V1(MapIONames[MapIO::V1],
                                Controller::GetV1RelativeValue(),
                                separator_width);
    res &= indicator_V1.Render(fb, &point);

    MapIOIndicator indicator_V2(MapIONames[MapIO::V2],
                                Controller::GetV2RelativeValue(),
                                separator_width);
    res &= indicator_V2.Render(fb, &point);

    MapIOIndicator indicator_V3(MapIONames[MapIO::V3],
                                Controller::GetV3RelativeValue(),
                                separator_width);
    res &= indicator_V3.Render(fb, &point);

    separator_width = 0;
    MapIOIndicator indicator_V4(MapIONames[MapIO::V4],
                                Controller::GetV4RelativeValue(),
                                separator_width);
    res &= indicator_V4.Render(fb, &point);

    res &= draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH);
    return res;
}