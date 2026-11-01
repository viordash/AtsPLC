#pragma once

#include "Display/display.h"
#include <stdint.h>
#include <unistd.h>

class ListBox {
  protected:
    const uint8_t left_padding = 4;
    const uint8_t top_padding = 2;

    const static int line_size = 21;
    const static int lines_count = 8;
    char title[line_size];
    char lines[lines_count][line_size];

    size_t max_view_rows_count;
    size_t items_count;
    int selected;
    int title_x;
    bool frame_buffer_req_render;
    int view_top_index;

    void BuildTitle(const char *title);

  public:
    explicit ListBox(const char *title);

    void Render(FrameBuffer *fb);
    bool Insert(int pos, const char *text);
    void Select(int index);
};
