#include "Display/ScrollBar.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

static const char *TAG_ScrollBar = "ScrollBar";

IRAM_ATTR void ScrollBar::Render(FrameBuffer *fb,
                                 uint16_t left,
                                 uint16_t top,
                                 uint16_t scroll_height,
                                 size_t count,
                                 size_t viewport_count,
                                 size_t view_topindex) {
    bool nothing_to_scroll = count <= viewport_count;
    if (nothing_to_scroll) {
        return;
    }

    uint16_t min_height = MAX((scroll_height / 16), 3);
    uint16_t height = ((size_t)scroll_height * viewport_count) / count;
    if (height < min_height) {
        height = min_height;
    }

    uint16_t y;

    uint16_t moving_area = scroll_height - height;
    uint16_t steps = count - viewport_count;
    if (moving_area >= steps) {
        uint16_t step_height = moving_area / steps;
        uint16_t y_offset = view_topindex * step_height;
        uint16_t round_tail = moving_area - (step_height * steps);
        height += round_tail;
        y = top + y_offset;
    } else {
        uint16_t step_mul = (steps * 10) / moving_area;
        uint16_t step_div = (steps * step_mul) / moving_area;
        uint16_t y_offset = (view_topindex * step_mul) / step_div;
        y = top + y_offset;
    }

    ESP_LOGD(TAG_ScrollBar,
             "Render: x:%u, y:%u, height:%u, top:%u, count:%u, viewport_count:%u, view_topindex:%u",
             (unsigned int)left,
             (unsigned int)y,
             (unsigned int)height,
             (unsigned int)top,
             (unsigned int)count,
             (unsigned int)viewport_count,
             (unsigned int)view_topindex);

    ASSERT(draw_vert_line(fb, left, y, height));
    ASSERT(draw_vert_line(fb, left + 1, y, height));
}
