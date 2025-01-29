#pragma once

#include "Display/Common.h"
#include "LogicProgram/MapIO.h"
#include <stdint.h>
#include <unistd.h>

class LogsList {
  protected:
    const static int line_size = 16;
    const static int lines_count = 5;
    char lines[lines_count][line_size];

    int curr_line;

  public:
    explicit LogsList();

    bool Render(uint8_t *fb);
    void Append(const char *message);
};
