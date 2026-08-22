#include "Display/LogsList.h"
#include "Display/display.h"
#include "lassert.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

LogsList::LogsList(const char *title) : ListBox(title) {
    curr_line = 0;
}

void LogsList::Append(const char *message) {
    char line[line_size];
    size_t len = strlen(message);

    if (len > line_size - 1) {
        const int elips_len = 3;
        strncpy(line, message, line_size - elips_len - 1);
        strcpy(&line[line_size - elips_len - 1], "...");
    } else {
        strcpy(line, message);
    }

    auto max_rows = MIN(max_view_rows_count, (size_t)lines_count);
    if ((size_t)curr_line < max_rows) {
        ASSERT(Insert(curr_line, line));
        curr_line++;
    } else {
        for (size_t i = 0; i < max_rows - 1; i++) {
            strncpy(lines[i], lines[i + 1], sizeof(lines[0]));
        }
        ASSERT(Insert(max_rows - 1, line));
    }
    frame_buffer_req_render = true;
}