#include "Display/LogsList.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    if (curr_line < lines_count) {
        strcpy(lines[curr_line], line);
        curr_line++;
    } else {
        for (size_t i = 0; i < lines_count - 1; i++) {
            strcpy(lines[i], lines[i + 1]);
        }
        strcpy(lines[lines_count - 1], line);
    }
}