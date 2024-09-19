#include "Display/SelectableElement.h"
#include "Display/bitmaps/element_cursor_0.h"
#include "Display/bitmaps/element_cursor_1.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SelectableElement = "SelectableElement";

SelectableElement::SelectableElement() {
    selected = false;
}

SelectableElement::~SelectableElement() {
}

void SelectableElement::ChangeSelection(bool selected) {
    ESP_LOGD(TAG_SelectableElement, "ChangeSelection, %u", selected);
    this->selected = selected;
}

bool SelectableElement::Selected() {
    return selected;
}

bool SelectableElement::Render(uint8_t *fb, Point *start_point) {
    if (!selected) {
        return true;
    }

    uint64_t curr_time = esp_timer_get_time();
    const Bitmap *bitmap;
    const int blink_timer_524ms = 0x80000;
    if (curr_time & blink_timer_524ms) {
        bitmap = &SelectableElement::bitmap_0;
    } else {
        bitmap = &SelectableElement::bitmap_1;
    }
    draw_bitmap(fb, start_point->x + 1, start_point->y + 1, bitmap);

    return true;
}