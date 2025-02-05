#pragma once

#include <stdint.h>
#include <unistd.h>

class ListBox {
  protected:
    const uint8_t left_padding = 4;
    const uint8_t top_padding = 2;

    const static int line_size = 21;
    const static int lines_count = 4;
    char title[line_size];
    char lines[lines_count][line_size];

    int selected;
    int title_x;

    void BuildTitle(const char *title);

  public:
    explicit ListBox(const char *title);

    bool Render(uint8_t *fb);
    bool Insert(int pos, const char *text);
    void Select(int index);
};
