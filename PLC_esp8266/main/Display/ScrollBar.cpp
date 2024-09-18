#include "Display/ScrollBar.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ScrollBar = "ScrollBar";

IRAM_ATTR bool
ScrollBar::Render(uint8_t *fb, size_t count, size_t viewport_count, size_t view_topindex) {
    bool res = true;

    bool nothing_to_scroll = count <= viewport_count;
    if (nothing_to_scroll) {
        return res;
    }
    uint8_t x = SCROLLBAR_LEFT;
    uint8_t scroll_steps = count - viewport_count;
    uint8_t height = SCROLLBAR_HEIGHT / (scroll_steps + 1);
    if (height < 3) {
        height = 3;
    }
    uint8_t moving_area = SCROLLBAR_HEIGHT - height;
    uint8_t scroll_step_height = moving_area / scroll_steps;

    uint8_t y_offset = view_topindex * scroll_step_height;
    uint8_t y = SCROLLBAR_TOP + y_offset;

    ESP_LOGD(TAG_ScrollBar,
             "Render: x:%u, y:%u, height:%u, top:%u, count:%u, viewport_count:%u, view_topindex:%u",
             x,
             y,
             height,
             SCROLLBAR_TOP,
             (uint32_t)count,
             (uint32_t)viewport_count,
             (uint32_t)view_topindex);

    res = draw_vert_line(fb, x, y, height);
    if (res) {
        res = draw_vert_line(fb, x + 1, y, height);
    }
    return res;
}
