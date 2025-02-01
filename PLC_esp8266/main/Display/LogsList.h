#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogsList {
  protected:
    const static int line_size = 21;
    const static int lines_count = 4;
    char title[line_size];
    char lines[lines_count][line_size];

    int curr_line;
    int title_x;

    void BuildTitle(const char *title);

  public:
    explicit LogsList(const char *title);

    bool Render(uint8_t *fb);
    void Append(const char *message);
};
