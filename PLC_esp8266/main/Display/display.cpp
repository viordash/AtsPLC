#include "Display/display.h"
#include "LogicProgram/LogicProgram.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <driver/i2c.h>
#include <esp_err.h>
extern "C" {
#include <ssd1306/ssd1306.h>
}

#define SCL_PIN GPIO_NUM_4
#define SDA_PIN GPIO_NUM_5

static const char *TAG_display = "display";

static struct {
    ssd1306_t dev = { .i2c_port = I2C_NUM_0,
                      .i2c_addr = SSD1306_I2C_ADDR_0,
                      .screen = SSD1306_SCREEN,
                      .width = DISPLAY_WIDTH,
                      .height = DISPLAY_HEIGHT };
    FrameBuffer frame_buffer;
    const font_info_t *font_4X7 = font_builtin_fonts[FONT_FACE_BITOCRA_4X7];
    const font_info_t *font_5X7 = font_builtin_fonts[FONT_FACE_GLCD5x7];
    const font_info_t *font_6X12 = font_builtin_fonts[FONT_FACE_TERMINUS_6X12_ISO8859_1];
    const font_info_t *font_8X14 = font_builtin_fonts[FONT_FACE_TERMINUS_8X14_ISO8859_1];
} display;

void display_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.clk_stretch_tick = 300;
    ESP_ERROR_CHECK(i2c_driver_install(display.dev.i2c_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(display.dev.i2c_port, &conf));

    if (ssd1306_init(&display.dev) != 0) {
        ESP_LOGW(TAG_display, "First init failed, trying to repeat init");
        ESP_ERROR_CHECK(ssd1306_init(&display.dev) != 0 ? ESP_FAIL : ESP_OK);
    }
    ESP_ERROR_CHECK(ssd1306_load_frame_buffer(&display.dev, NULL) != 0 ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(ssd1306_set_whole_display_lighting(&display.dev, false) != 0 ? ESP_FAIL
                                                                                 : ESP_OK);

    ESP_LOGI(TAG_display, "init succesfully");
}

IRAM_ATTR FrameBuffer *begin_render() {
    memset(display.frame_buffer.buffer, 0, sizeof(display.frame_buffer.buffer));
    display.frame_buffer.has_changes = false;
    return &display.frame_buffer;
}
IRAM_ATTR void end_render(FrameBuffer *fb) {
    ssd1306_load_frame_buffer(&display.dev, fb->buffer);
}

IRAM_ATTR int draw_text_f4X7(FrameBuffer *fb, uint8_t x, uint8_t y, const char *text) {
    return ssd1306_draw_string(&display.dev,
                               fb->buffer,
                               display.font_4X7,
                               x,
                               y,
                               text,
                               OLED_COLOR_WHITE,
                               OLED_COLOR_BLACK);
}

IRAM_ATTR int draw_text_f5X7(FrameBuffer *fb, uint8_t x, uint8_t y, const char *text) {
    return ssd1306_draw_string(&display.dev,
                               fb->buffer,
                               display.font_5X7,
                               x,
                               y,
                               text,
                               OLED_COLOR_WHITE,
                               OLED_COLOR_BLACK);
}

IRAM_ATTR int draw_text_f6X12(FrameBuffer *fb, uint8_t x, uint8_t y, const char *text) {
    return ssd1306_draw_string(&display.dev,
                               fb->buffer,
                               display.font_6X12,
                               x,
                               y,
                               text,
                               OLED_COLOR_WHITE,
                               OLED_COLOR_BLACK);
}

IRAM_ATTR int
draw_text_f6X12_colored(FrameBuffer *fb, uint8_t x, uint8_t y, const char *text, bool inverse) {
    return ssd1306_draw_string(&display.dev,
                               fb->buffer,
                               display.font_6X12,
                               x,
                               y,
                               text,
                               inverse ? OLED_COLOR_BLACK : OLED_COLOR_WHITE,
                               inverse ? OLED_COLOR_WHITE : OLED_COLOR_BLACK);
}

IRAM_ATTR int draw_text_f8X14(FrameBuffer *fb, uint8_t x, uint8_t y, const char *text) {
    return ssd1306_draw_string(&display.dev,
                               fb->buffer,
                               display.font_8X14,
                               x,
                               y,
                               text,
                               OLED_COLOR_WHITE,
                               OLED_COLOR_BLACK);
}

IRAM_ATTR int get_text_f4X7_height() {
    return display.font_4X7->height;
}
IRAM_ATTR int get_text_f4X7_width() {
    return display.font_4X7->char_descriptors[0].width;
}
IRAM_ATTR int get_text_f5X7_height() {
    return display.font_5X7->height;
}
IRAM_ATTR int get_text_f6X12_height() {
    return display.font_6X12->height;
}
IRAM_ATTR int get_text_f6X12_width() {
    return display.font_6X12->char_descriptors[0].width;
}
IRAM_ATTR int get_text_f8X14_height() {
    return display.font_8X14->height;
}

IRAM_ATTR bool draw_active_network(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t w) {
    int err = 0;
    if (w > 0) {
        err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y, w, OLED_COLOR_WHITE);
        if (err == 0) {
            err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y + 1, w, OLED_COLOR_WHITE);
        }
    }
    return err == 0;
}

IRAM_ATTR bool
draw_passive_network(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t w, bool inverse_dash) {
    int err = 0;
    ssd1306_color_t color_top_line = inverse_dash ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
    ssd1306_color_t color_bottom_line = inverse_dash ? OLED_COLOR_BLACK : OLED_COLOR_WHITE;
    while (w >= 2) {
        if (err == 0) {
            err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y, 2, color_top_line);
        }
        if (err == 0) {
            err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y + 1, 2, color_bottom_line);
        }
        w -= 2;
        x += 2;
        color_top_line = color_top_line == OLED_COLOR_BLACK ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
        color_bottom_line =
            color_bottom_line == OLED_COLOR_BLACK ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
    }
    return err == 0;
}

IRAM_ATTR bool draw_active_income_rail(FrameBuffer *fb, uint8_t x, uint8_t y) {
    int err;
    y -= INCOME_RAIL_NETWORK_TOP;
    err = ssd1306_draw_vline(&display.dev, fb->buffer, x, y, INCOME_RAIL_HEIGHT, OLED_COLOR_WHITE);
    if (err == 0 && INCOME_RAIL_WIDTH == 2) {
        ssd1306_draw_vline(&display.dev,
                           fb->buffer,
                           x + 1,
                           y,
                           INCOME_RAIL_HEIGHT,
                           OLED_COLOR_WHITE);
    }
    return err == 0;
}

IRAM_ATTR bool draw_passive_income_rail(FrameBuffer *fb, uint8_t x, uint8_t y) {
    int err;
    y -= INCOME_RAIL_NETWORK_TOP;
    uint8_t last_y = y + INCOME_RAIL_HEIGHT;
    err = ssd1306_draw_vline(&display.dev, fb->buffer, x, y, INCOME_RAIL_HEIGHT, OLED_COLOR_WHITE);
    if (err == 0 && INCOME_RAIL_WIDTH == 2) {
        int dashed_line_height = INCOME_RAIL_HEIGHT / 3;

        uint8_t height = INCOME_RAIL_HEIGHT / 3;
        err = ssd1306_draw_vline(&display.dev, fb->buffer, x + 1, y, height, OLED_COLOR_WHITE);
        y += height;

        ssd1306_color_t color_line = OLED_COLOR_WHITE;
        while (err == 0 && dashed_line_height >= 2) {
            err = ssd1306_draw_vline(&display.dev, fb->buffer, x + 1, y, 2, color_line);
            dashed_line_height -= 2;
            y += 2;
            color_line = color_line == OLED_COLOR_BLACK ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
        }

        if (err == 0) {
            height = last_y - y;
            ssd1306_draw_vline(&display.dev, fb->buffer, x + 1, y, height, OLED_COLOR_WHITE);
        }
    }
    return err == 0;
}

IRAM_ATTR bool draw_outcome_rail(FrameBuffer *fb, uint8_t x, uint8_t y) {
    int err;
    y -= OUTCOME_RAIL_NETWORK_TOP;
    uint8_t height = OUTCOME_RAIL_HEIGHT;
    err = ssd1306_draw_vline(&display.dev, fb->buffer, x, y, height, OLED_COLOR_WHITE);
    if (err == 0) {
        err = ssd1306_draw_vline(&display.dev, fb->buffer, x + 1, y, height, OLED_COLOR_WHITE);
    }
    return err == 0;
}

IRAM_ATTR bool draw_vert_progress_bar(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t percent04) {
    int err = 0;
    int height = (VERT_PROGRESS_BAR_HEIGHT * percent04) / 250;
    if (height > 0) {
        uint8_t y_pos = y + (VERT_PROGRESS_BAR_HEIGHT - height);
        err = ssd1306_draw_vline(&display.dev, fb->buffer, x + 0, y_pos, height, OLED_COLOR_WHITE);
        if (err == 0) {
            err = ssd1306_draw_vline(&display.dev,
                                     fb->buffer,
                                     x + 1,
                                     y_pos,
                                     height,
                                     OLED_COLOR_WHITE);
        }
    }
    if (err == 0) {
        err = ssd1306_draw_vline(&display.dev,
                                 fb->buffer,
                                 x + 2,
                                 y,
                                 VERT_PROGRESS_BAR_HEIGHT,
                                 OLED_COLOR_WHITE);
    }
    return err == 0;
}

IRAM_ATTR bool draw_horz_progress_bar(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t percent04) {
    int err = 0;
    int width = (HORZ_PROGRESS_BAR_WIDTH * percent04) / 250;
    if (width > 0) {
        err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y + 0, width, OLED_COLOR_WHITE);
        if (err == 0) {
            err = ssd1306_draw_hline(&display.dev, fb->buffer, x, y + 1, width, OLED_COLOR_WHITE);
        }
    }
    if (err == 0) {
        // err = ssd1306_draw_hline(&display.dev, fb, x, y + 2, HORZ_PROGRESS_BAR_WIDTH, OLED_COLOR_WHITE);
    }
    return err == 0;
}

IRAM_ATTR bool draw_vert_line(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t h) {
    return ssd1306_draw_vline(&display.dev, fb->buffer, x, y, h, OLED_COLOR_WHITE) == 0;
}

IRAM_ATTR bool draw_horz_line(FrameBuffer *fb, uint8_t x, uint8_t y, uint8_t w) {
    return ssd1306_draw_hline(&display.dev, fb->buffer, x, y, w, OLED_COLOR_WHITE) == 0;
}

IRAM_ATTR bool draw_rectangle(FrameBuffer *fb, int8_t x, int8_t y, uint8_t w, uint8_t h) {
    return ssd1306_draw_rectangle(&display.dev, fb->buffer, x, y, w, h, OLED_COLOR_WHITE) == 0;
}

IRAM_ATTR void draw_bitmap(FrameBuffer *fb, uint8_t x, uint8_t y, const struct Bitmap *bitmap) {
    for (int row = y; row < y + bitmap->size.height; row += 8) {
        if (row >= DISPLAY_HEIGHT) {
            continue;
        }
        for (int column = x; column < x + bitmap->size.width; column++) {
            if (column >= DISPLAY_WIDTH) {
                continue;
            }
            int src_id = (((row - y) / 8) * bitmap->size.width) + (column - x);
            int dst_id = ((row / 8) * DISPLAY_WIDTH) + column;

            uint8_t b = bitmap->data[src_id];
            fb->buffer[dst_id] |= b << (y % 8);
            if ((y % 8) > 0 && row + 1 < DISPLAY_HEIGHT) {
                fb->buffer[dst_id + DISPLAY_WIDTH] |= b >> (8 - (y % 8));
            }
        }
    }
}
