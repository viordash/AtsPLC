#include "Display/ListBox.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ListBox = "ListBox";

ListBox::ListBox(const char *title) {
    memset(lines, 0, sizeof(lines));
    selected = -1;

    BuildTitle(title);
    frame_buffer_req_render = false;
}

void ListBox::BuildTitle(const char *title) {
    size_t len = strlen(title);
    const int left_decor_size = 2;
    const int right_decor_size = 2;

    strcpy(this->title, "> ");
    if (len > line_size - (left_decor_size + right_decor_size) - 1) {
        strncat(this->title, title, line_size - (left_decor_size + right_decor_size) - 1);
    } else {
        strcat(this->title, title);
    }
    strcat(this->title, " <");

    auto fb = new FrameBuffer();
    const uint8_t x = 2;
    const uint8_t y = 2;
    int width = draw_text_f6X12(fb, x, y, this->title);
    ASSERT(width > 0);
    title_x = x + (DISPLAY_WIDTH - width) / 2;
    ASSERT(title_x > 0);
    delete fb;
}

void ListBox::Render(FrameBuffer *fb) {
    const uint8_t x = left_padding;
    uint8_t y = top_padding;
    uint8_t height = get_text_f6X12_height();

    ASSERT(draw_rectangle(fb, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT));
    ASSERT(draw_text_f6X12(fb, title_x, y, title) > 0);
    y += height;
    for (int i = 0; i < lines_count; i++) {
        ASSERT(draw_text_f6X12_colored(fb, x, y + height * i, lines[i], selected == i) >= 0);
    }
    fb->has_changes |= frame_buffer_req_render;
}

bool ListBox::Insert(int pos, const char *text) {
    if (pos < 0 || pos >= lines_count) {
        return false;
    }

    char *line = lines[pos];
    size_t len = strlen(text);

    if (len > line_size - 1) {
        strncpy(line, text, line_size - 1);
        line[line_size - 1] = 0;
    } else {
        strcpy(line, text);
    }
    frame_buffer_req_render = true;
    return true;
}

void ListBox::Select(int index) {
    selected = index;
    frame_buffer_req_render = true;
    ESP_LOGI(TAG_ListBox, "Select:%d", index);
}