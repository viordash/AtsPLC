#include "LogicProgram/StatusBar.h"
#include "Display/Common.h"
#include "Display/MapIOIndicator.h"
#include "LogicProgram/MapIO.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatusBar::StatusBar(const Controller &controller, uint8_t y)
    : LogicItemBase(controller), DisplayItemBase() {
    this->y = y;
}

StatusBar::~StatusBar() {
}

bool StatusBar::DoAction() {
    return true;
}

void StatusBar::Render(uint8_t *fb) {
    uint8_t separator_width = 1;
    Point point = { 0, y };
    MapIOIndicator indicator_AI(point,
                                MapIONames[MapIO::AI],
                                controller.GetAIRelativeValue(),
                                separator_width);
    indicator_AI.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_DI(point,
                                MapIONames[MapIO::DI],
                                controller.GetDIRelativeValue(),
                                separator_width);
    indicator_DI.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_O1(point,
                                MapIONames[MapIO::O1],
                                controller.GetO1RelativeValue(),
                                separator_width);
    indicator_O1.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_O2(point,
                                MapIONames[MapIO::O2],
                                controller.GetO2RelativeValue(),
                                separator_width);
    indicator_O2.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V1(point,
                                MapIONames[MapIO::V1],
                                controller.GetV1RelativeValue(),
                                separator_width);
    indicator_V1.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V2(point,
                                MapIONames[MapIO::V2],
                                controller.GetV2RelativeValue(),
                                separator_width);
    indicator_V2.Render(fb);

    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V3(point,
                                MapIONames[MapIO::V3],
                                controller.GetV3RelativeValue(),
                                separator_width);
    indicator_V3.Render(fb);

    separator_width = 0;
    point.x += MapIOIndicator::GetWidth() + separator_width;
    MapIOIndicator indicator_V4(point,
                                MapIONames[MapIO::V4],
                                controller.GetV4RelativeValue(),
                                separator_width);
    indicator_V4.Render(fb);

    draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH);
}