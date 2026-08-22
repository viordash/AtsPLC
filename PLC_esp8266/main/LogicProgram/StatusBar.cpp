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

StatusBar::StatusBar(uint8_t y)
    : y{ y },
      indicators{ MapIOIndicator(MapIO::DI), MapIOIndicator(MapIO::AI), MapIOIndicator(MapIO::O1),
                  MapIOIndicator(MapIO::O2), MapIOIndicator(MapIO::V1), MapIOIndicator(MapIO::V2),
                  MapIOIndicator(MapIO::V3), MapIOIndicator(MapIO::V4) } {
}

StatusBar::~StatusBar() {
}

IRAM_ATTR void
StatusBar::RenderIndicator(FrameBuffer *fb, Point *point, const MapIO io_adr, uint8_t value) {
    const uint8_t separator_width = 1;
    indicators[io_adr].Render(fb, point, value);
    point->x += separator_width;
}

IRAM_ATTR void StatusBar::Render(FrameBuffer *fb) {
    Point point = { 2, y };

    RenderIndicator(fb, &point, MapIO::AI, Controller::AI.PeekValue());
    RenderIndicator(fb, &point, MapIO::DI, Controller::DI.PeekValue());
    RenderIndicator(fb, &point, MapIO::O1, Controller::O1.PeekValue());
    RenderIndicator(fb, &point, MapIO::O2, Controller::O2.PeekValue());
    RenderIndicator(fb, &point, MapIO::V1, Controller::V1.PeekValue());
    RenderIndicator(fb, &point, MapIO::V2, Controller::V2.PeekValue());
    RenderIndicator(fb, &point, MapIO::V3, Controller::V3.PeekValue());
    RenderIndicator(fb, &point, MapIO::V4, Controller::V4.PeekValue());

    ASSERT(draw_horz_line(fb, 0, y + MapIOIndicator::GetHeight(), DISPLAY_WIDTH));
}