#include "LogicProgram/StatusBar.h"
#include "Display/display.h"
#include "Display/MapIOIndicator.h"
#include "LogicProgram/MapIO.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatusBar::StatusBar(const Controller *controller, uint8_t y) {
    this->y = y;
}

StatusBar::~StatusBar() {
}

bool StatusBar::Render(uint8_t *fb) {
    bool res = true;
    uint8_t separator_width = 1;
    Point point = { 2, y };
    MapIOIndicator indicator_AI(point,
                                MapIONames[MapIO::AI],
                                Controller::GetAIRelativeValue(),
                                separator_width);
    res &= indicator_AI.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_DI(point,
                                MapIONames[MapIO::DI],
                                Controller::GetDIRelativeValue(),
                                separator_width);
    res &= indicator_DI.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_O1(point,
                                MapIONames[MapIO::O1],
                                Controller::GetO1RelativeValue(),
                                separator_width);
    res &= indicator_O1.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_O2(point,
                                MapIONames[MapIO::O2],
                                Controller::GetO2RelativeValue(),
                                separator_width);
    res &= indicator_O2.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V1(point,
                                MapIONames[MapIO::V1],
                                Controller::GetV1RelativeValue(),
                                separator_width);
    res &= indicator_V1.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V2(point,
                                MapIONames[MapIO::V2],
                                Controller::GetV2RelativeValue(),
                                separator_width);
    res &= indicator_V2.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V3(point,
                                MapIONames[MapIO::V3],
                                Controller::GetV3RelativeValue(),
                                separator_width);
    res &= indicator_V3.Render(fb);

    separator_width = 0;
    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V4(point,
                                MapIONames[MapIO::V4],
                                Controller::GetV4RelativeValue(),
                                separator_width);
    res &= indicator_V4.Render(fb);

    res &= draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH);
    return res;
}