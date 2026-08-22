#include "Display/ListBox.h"
#include "Display/ScrollBar.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ListBox = "ListBox";
static const uint16_t frame_size = 2;

ListBox::ListBox(const char *title)
    : max_view_rows_count{ 0 }, items_count{ 0 }, selected{ -1 }, title_x{ 0 },
      frame_buffer_req_render{ false }, view_top_index{ 0 } {
    memset(lines, 0, sizeof(lines));

    BuildTitle(title);

    const uint16_t title_height = get_text_f6X12_height();
    max_view_rows_count =
        (DISPLAY_HEIGHT - top_padding - frame_size - title_height) / get_text_f6X12_height();
    ASSERT(max_view_rows_count > 0);
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

    for (size_t row = 0; row < max_view_rows_count; row++) {
        size_t index = view_top_index + row;
        if (index >= items_count) {
            break;
        }
        ASSERT(draw_text_f6X12_colored(fb, x, y + height * row, lines[index], selected == (int)index)
               >= 0);
    }

    ScrollBar::Render(fb,
                      DISPLAY_WIDTH - (frame_size + SCROLLBAR_WIDTH),
                      y,
                      max_view_rows_count * height,
                      items_count,
                      max_view_rows_count,
                      view_top_index);

    fb->has_changes |= frame_buffer_req_render;
    frame_buffer_req_render = false;
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

    if ((size_t)pos >= items_count) {
        items_count = pos + 1;
    }
    frame_buffer_req_render = true;
    return true;
}

void ListBox::Select(int index) {
    ASSERT(index >= 0 && (size_t)index < items_count);
    selected = index;
    frame_buffer_req_render = true;
    ESP_LOGI(TAG_ListBox, "Select:%d", index);

    if (selected < view_top_index) {
        view_top_index = selected;
    } else if (selected >= view_top_index + (int)max_view_rows_count) {
        view_top_index = selected - (int)max_view_rows_count + 1;
    }
}