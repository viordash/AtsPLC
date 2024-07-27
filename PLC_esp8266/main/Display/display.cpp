#include "Display/display.h"
#include "Display/DisplayItemBase.h"
#include "Display/demo.h"
#include "LogicProgram/InputNO.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
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

static const char *TAG = "display";

static struct {
    ssd1306_t dev = { .i2c_port = I2C_NUM_0,
                      .i2c_addr = SSD1306_I2C_ADDR_0,
                      .screen = SSD1306_SCREEN,
                      .width = DISPLAY_WIDTH,
                      .height = DISPLAY_HEIGHT };
    uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

    const font_info_t *font_4X7 = font_builtin_fonts[FONT_FACE_BITOCRA_4X7];
    const font_info_t *font_5X7 = font_builtin_fonts[FONT_FACE_GLCD5x7];
    const font_info_t *font_6X12 = font_builtin_fonts[FONT_FACE_TERMINUS_6X12_ISO8859_1];
    const font_info_t *font_8X14 = font_builtin_fonts[FONT_FACE_TERMINUS_8X14_ISO8859_1];
} display;

// const font_info_t *font = font_builtin_fonts[FONT_FACE_GLCD5x7];

void ladder_diagram(int8_t x, int8_t y);
void ladder_diagram_acsii(int8_t x, int8_t y);

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

    ESP_ERROR_CHECK(ssd1306_init(&display.dev) != 0 ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(ssd1306_load_frame_buffer(&display.dev, NULL) != 0 ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(ssd1306_set_whole_display_lighting(&display.dev, false) != 0 ? ESP_FAIL
                                                                                 : ESP_OK);

    ssd1306_load_frame_buffer(&display.dev, bitmap_demo_0);

    // ssd1306_draw_string(&display.dev,
    //                     display.buffer,
    //                     display.font_4X7,
    //                     0,
    //                     0,
    //                     "001.IN0 ? OUT0<1 : OUT1<1",
    //                     OLED_COLOR_WHITE,
    //                     OLED_COLOR_BLACK);

    // ladder_diagram(16, 12);
    // ladder_diagram(16, 29);
    // ladder_diagram(16, 46);
    // // ladder_diagram_acsii(14, 32);

    // ESP_ERROR_CHECK(ssd1306_load_frame_buffer(&display.dev, display.buffer) != 0 ? ESP_FAIL
    //                                                                              : ESP_OK);
    ESP_LOGI(TAG, "init succesfully");
}

void ladder_diagram(int8_t x, int8_t y) {

    ssd1306_draw_vline(&display.dev, display.buffer, x, y, 17, OLED_COLOR_WHITE);
    ssd1306_draw_hline(&display.dev, display.buffer, x + 1, y + 11 + 1, 18, OLED_COLOR_WHITE);

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 12,
                      y + 0,
                      '0',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_vline(&display.dev, display.buffer, x + 19, y + 7 + 1, 9, OLED_COLOR_WHITE);
    ssd1306_draw_vline(&display.dev, display.buffer, x + 24, y + 7 + 1, 9, OLED_COLOR_WHITE);

    ssd1306_draw_hline(&display.dev, display.buffer, x + 25, y + 11 + 1, 11, OLED_COLOR_WHITE);

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 29,
                      y + 0,
                      '1',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_vline(&display.dev, display.buffer, x + 36, y + 7 + 1, 9, OLED_COLOR_WHITE);
    ssd1306_draw_line(&display.dev,
                      display.buffer,
                      x + 36,
                      y + 15 + 1,
                      x + 41,
                      y + 7 + 1,
                      OLED_COLOR_WHITE);
    ssd1306_draw_vline(&display.dev, display.buffer, x + 41, y + 7 + 1, 9, OLED_COLOR_WHITE);

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 46 + 5,
                      y + 0,
                      'A',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_hline(&display.dev, display.buffer, x + 42, y + 11 + 1, 11 + 5, OLED_COLOR_WHITE);
    ssd1306_draw_circle(&display.dev, display.buffer, x + 57 + 5, y + 11 + 1, 5, OLED_COLOR_WHITE);
    ssd1306_draw_hline(&display.dev, display.buffer, x + 62 + 5, y + 11 + 1, 7, OLED_COLOR_WHITE);
    ssd1306_draw_vline(&display.dev, display.buffer, x + 74, y, 17, OLED_COLOR_WHITE);
}

void ladder_diagram_acsii(int8_t x, int8_t y) {

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 21,
                      y + 0,
                      '0',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 49,
                      y + 0,
                      '1',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_char(&display.dev,
                      display.buffer,
                      display.font_6X12,
                      x + 77,
                      y + 0,
                      'A',
                      OLED_COLOR_WHITE,
                      OLED_COLOR_BLACK);

    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_8X14,
                        x,
                        y + 12,
                        "\x15\x12\x16\x15\x12\x16/\x15\x12()\x12\x16",
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);
}

uint8_t *get_display_buffer() {
    return display.buffer;
}

void begin_render() {
    memset(display.buffer, 0, sizeof(display.buffer));
}
void end_render() {

    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_4X7,
                        0,
                        0,
                        "DI V2",
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_5X7,
                        30,
                        0,
                        "DI V2",
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_6X12,
                        80,
                        0,
                        "DI V2",
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    ssd1306_load_frame_buffer(&display.dev, display.buffer);
}

void draw_text_f5X7(uint8_t x, uint8_t y, const char *text) {
    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_5X7,
                        x,
                        y,
                        text,
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);
}

void draw_text_f6X12(uint8_t x, uint8_t y, const char *text) {
    ssd1306_draw_string(&display.dev,
                        display.buffer,
                        display.font_6X12,
                        x,
                        y,
                        text,
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);
}

void draw_active_network(int8_t x, int8_t y, uint8_t w) {
    ssd1306_draw_hline(&display.dev, display.buffer, x, y, w, OLED_COLOR_WHITE);
    ssd1306_draw_hline(&display.dev, display.buffer, x, y + 1, w, OLED_COLOR_WHITE);
}

void draw_passive_network(int8_t x, int8_t y, uint8_t w, bool inverse_dash) {
    ssd1306_color_t color_top_line = inverse_dash ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
    ssd1306_color_t color_bottom_line = inverse_dash ? OLED_COLOR_BLACK : OLED_COLOR_WHITE;
    while (w >= 2) {
        ssd1306_draw_hline(&display.dev, display.buffer, x, y, 2, color_top_line);
        ssd1306_draw_hline(&display.dev, display.buffer, x, y + 1, 2, color_bottom_line);
        w -= 2;
        x += 2;
        color_top_line = color_top_line == OLED_COLOR_BLACK ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
        color_bottom_line =
            color_bottom_line == OLED_COLOR_BLACK ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
    }
}

void draw_bitmap(uint8_t *fb, uint8_t x, uint8_t y, const struct Bitmap *bitmap) {
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
            fb[dst_id] |= b << (y % 8);
            if ((y % 8) > 0 && row + 1 < DISPLAY_HEIGHT) {
                fb[dst_id + DISPLAY_WIDTH] |= b >> (8 - (y % 8));
            }
        }
    }
}

void display_demo(int8_t x, int8_t y) {
    memset(display.buffer, 0, sizeof(display.buffer));
    draw_bitmap(display.buffer, x, y, &cmp_greate_or_equal_active);
    ssd1306_load_frame_buffer(&display.dev, display.buffer);
}