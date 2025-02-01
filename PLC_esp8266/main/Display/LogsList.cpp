#include "Display/LogsList.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LogsList::LogsList(const char *title) {
    curr_line = 0;
    memset(lines, 0, sizeof(lines));

    BuildTitle(title);
}

void LogsList::BuildTitle(const char *title) {
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

    uint8_t *fb = new uint8_t[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
    const uint8_t x = 2;
    const uint8_t y = 2;
    int width = draw_text_f6X12(fb, x, y, this->title);
    ESP_ERROR_CHECK(width <= 0);
    title_x = x + (DISPLAY_WIDTH - width) / 2;
    ESP_ERROR_CHECK(title_x <= 0);
    delete[] fb;
}

bool LogsList::Render(uint8_t *fb) {
    const uint8_t x = 4;
    uint8_t y = 2;
    uint8_t height = get_text_f6X12_height();

    if (!draw_rectangle(fb, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT)) {
        return false;
    }

    if (draw_text_f6X12(fb, title_x, y, title) <= 0) {
        return false;
    }
    y += height;
    for (size_t i = 0; i < lines_count; i++) {
        if (strlen(lines[i]) > 0 && draw_text_f6X12(fb, x, y + height * i, lines[i]) <= 0) {
            return false;
        }
    }
    return true;
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