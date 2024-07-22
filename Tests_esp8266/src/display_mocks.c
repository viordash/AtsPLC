#include "CppUTestExt/MockSupport_c.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "freertos/event_groups.h"
#include <stdlib.h>

#include <ssd1306/ssd1306.h>

#include "main/Display/fonts/fonts.c"

int ssd1306_load_frame_buffer(const ssd1306_t *dev, uint8_t buf[]) {
    (void)dev;
    (void)buf;
    return 0;
};

esp_err_t i2c_driver_install(i2c_port_t i2c_num, i2c_mode_t mode) {
    (void)i2c_num;
    (void)mode;

    return ESP_OK;
}

esp_err_t i2c_param_config(i2c_port_t i2c_num, const i2c_config_t *i2c_conf) {
    (void)i2c_num;
    (void)i2c_conf;

    return ESP_OK;
}

int ssd1306_init(const ssd1306_t *dev) {
    (void)dev;
    return 0;
}

int ssd1306_set_whole_display_lighting(const ssd1306_t *dev, bool light) {
    (void)dev;
    (void)light;
    return 0;
}

int ssd1306_draw_hline(const ssd1306_t *dev,
                       uint8_t *fb,
                       int8_t x,
                       int8_t y,
                       uint8_t w,
                       ssd1306_color_t color) {
    (void)dev;
    (void)fb;
    (void)x;
    (void)y;
    (void)w;
    (void)color;
    return 0;
}

int ssd1306_draw_vline(const ssd1306_t *dev,
                       uint8_t *fb,
                       int8_t x,
                       int8_t y,
                       uint8_t h,
                       ssd1306_color_t color) {
    (void)dev;
    (void)fb;
    (void)x;
    (void)y;
    (void)h;
    (void)color;
    return 0;
}

int ssd1306_draw_line(const ssd1306_t *dev,
                      uint8_t *fb,
                      int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      ssd1306_color_t color) {
    (void)dev;
    (void)fb;
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
    return 0;
}

int ssd1306_draw_char(const ssd1306_t *dev,
                      uint8_t *fb,
                      const font_info_t *font,
                      uint8_t x,
                      uint8_t y,
                      char c,
                      ssd1306_color_t foreground,
                      ssd1306_color_t background) {
    (void)dev;
    (void)fb;
    (void)font;
    (void)x;
    (void)y;
    (void)c;
    (void)foreground;
    (void)background;
    return 0;
}

int ssd1306_draw_string(const ssd1306_t *dev,
                        uint8_t *fb,
                        const font_info_t *font,
                        uint8_t x,
                        uint8_t y,
                        const char *str,
                        ssd1306_color_t foreground,
                        ssd1306_color_t background) {
    (void)dev;
    (void)fb;
    (void)font;
    (void)x;
    (void)y;
    (void)str;
    (void)foreground;
    (void)background;
    return 0;
}

int ssd1306_draw_circle(const ssd1306_t *dev,
                        uint8_t *fb,
                        int8_t x0,
                        int8_t y0,
                        uint8_t r,
                        ssd1306_color_t color) {
    (void)dev;
    (void)fb;
    (void)x0;
    (void)y0;
    (void)r;
    (void)color;
    return 0;
}