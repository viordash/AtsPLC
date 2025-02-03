#include "freertos/event_groups.h"
#include "CppUTestExt/MockSupport_c.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include <stdlib.h>

#include <ssd1306/ssd1306.h>

#include "main/Display/fonts/fonts.c"

#define abs(x) ((x) < 0 ? -(x) : (x))
#define swap(x, y)                                                                                 \
    do {                                                                                           \
        typeof(x) temp##x##y = x;                                                                  \
        x = y;                                                                                     \
        y = temp##x##y;                                                                            \
    } while (0)

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

int ssd1306_draw_pixel(const ssd1306_t *dev,
                       uint8_t *fb,
                       int8_t x,
                       int8_t y,
                       ssd1306_color_t color) {
    uint16_t index;

    if ((x >= dev->width) || (x < 0) || (y >= dev->height) || (y < 0))
        return -EINVAL;
    index = x + (y / 8) * dev->width;
    switch (color) {
        case OLED_COLOR_WHITE:
            fb[index] |= (1 << (y & 7));
            break;
        case OLED_COLOR_BLACK:
            fb[index] &= ~(1 << (y & 7));
            break;
        case OLED_COLOR_INVERT:
            fb[index] ^= (1 << (y & 7));
            break;
        default:
            break;
    }
    return 0;
}

int ssd1306_draw_hline(const ssd1306_t *dev,
                       uint8_t *fb,
                       int8_t x,
                       int8_t y,
                       uint8_t w,
                       ssd1306_color_t color) {
    uint16_t index;
    uint8_t mask, t;

    // boundary check
    if ((x >= dev->width) || (x < 0) || (y >= dev->height) || (y < 0))
        return -EINVAL;
    if (w == 0)
        return -EINVAL;
    if (x + w > dev->width)
        w = dev->width - x;

    t = w;
    index = x + (y / 8) * dev->width;
    mask = 1 << (y & 7);
    switch (color) {
        case OLED_COLOR_WHITE:
            while (t--) {
                fb[index] |= mask;
                ++index;
            }
            break;
        case OLED_COLOR_BLACK:
            mask = ~mask;
            while (t--) {
                fb[index] &= mask;
                ++index;
            }
            break;
        case OLED_COLOR_INVERT:
            while (t--) {
                fb[index] ^= mask;
                ++index;
            }
            break;
        default:
            break;
    }
    return 0;
}

int ssd1306_draw_vline(const ssd1306_t *dev,
                       uint8_t *fb,
                       int8_t x,
                       int8_t y,
                       uint8_t h,
                       ssd1306_color_t color) {
    uint16_t index;
    uint8_t mask, mod, t;

    // boundary check
    if ((x >= dev->width) || (x < 0) || (y >= dev->height) || (y < 0))
        return -EINVAL;
    if (h == 0)
        return -EINVAL;
    if (y + h > dev->height)
        h = dev->height - y;

    t = h;
    index = x + (y / 8) * dev->width;
    mod = y & 7;
    if (mod) // partial line that does not fit into byte at top
    {
        // Magic from Adafruit
        mod = 8 - mod;
        static const uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        mask = premask[mod];
        if (t < mod)
            mask &= (0xFF >> (mod - t));
        switch (color) {
            case OLED_COLOR_WHITE:
                fb[index] |= mask;
                break;
            case OLED_COLOR_BLACK:
                fb[index] &= ~mask;
                break;
            case OLED_COLOR_INVERT:
                fb[index] ^= mask;
                break;
            default:
                break;
        }

        if (t < mod)
            return 0;
        t -= mod;
        index += dev->width;
    }
    if (t >= 8) // byte aligned line at middle
    {
        switch (color) {
            case OLED_COLOR_WHITE:
                do {
                    fb[index] = 0xff;
                    index += dev->width;
                    t -= 8;
                } while (t >= 8);
                break;
            case OLED_COLOR_BLACK:
                do {
                    fb[index] = 0x00;
                    index += dev->width;
                    t -= 8;
                } while (t >= 8);
                break;
            case OLED_COLOR_INVERT:
                do {
                    fb[index] = ~fb[index];
                    index += dev->width;
                    t -= 8;
                } while (t >= 8);
                break;
            default:
                break;
        }
    }
    if (t) // partial line at bottom
    {
        mod = t & 7;
        static const uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
        mask = postmask[mod];
        switch (color) {
            case OLED_COLOR_WHITE:
                fb[index] |= mask;
                break;
            case OLED_COLOR_BLACK:
                fb[index] &= ~mask;
                break;
            case OLED_COLOR_INVERT:
                fb[index] ^= mask;
                break;
            default:
                break;
        }
    }
    return 0;
}

int ssd1306_draw_line(const ssd1306_t *dev,
                      uint8_t *fb,
                      int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      ssd1306_color_t color) {
    if ((x0 >= dev->width) || (x0 < 0) || (y0 >= dev->height) || (y0 < 0))
        return -EINVAL;
    if ((x1 >= dev->width) || (x1 < 0) || (y1 >= dev->height) || (y1 < 0))
        return -EINVAL;

    int err;
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t errx = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            if ((err = ssd1306_draw_pixel(dev, fb, y0, x0, color)))
                return err;
        } else {
            if ((err = ssd1306_draw_pixel(dev, fb, x0, y0, color)))
                return err;
        }
        errx -= dy;
        if (errx < 0) {
            y0 += ystep;
            errx += dx;
        }
    }
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
    uint8_t i, j;
    const uint8_t *bitmap;
    uint8_t line = 0;
    int err = 0;

    if (font == NULL)
        return 0;

    const font_char_desc_t *d = font_get_char_desc(font, c);
    if (d == NULL)
        return 0;

    bitmap = font->bitmap + d->offset;
    for (j = 0; j < font->height; ++j) {
        for (i = 0; i < d->width; ++i) {
            if (i % 8 == 0) {
                line = bitmap[(d->width + 7) / 8 * j + i / 8]; // line data
            }
            if (line & 0x80) {
                err = ssd1306_draw_pixel(dev, fb, x + i, y + j, foreground);
            } else {
                switch (background) {
                    case OLED_COLOR_TRANSPARENT:
                        // Not drawing for transparent background
                        break;
                    case OLED_COLOR_WHITE:
                    case OLED_COLOR_BLACK:
                        err = ssd1306_draw_pixel(dev, fb, x + i, y + j, background);
                        break;
                    case OLED_COLOR_INVERT:
                        // I don't know why I need invert background
                        break;
                }
            }
            if (err)
                return -ERANGE;
            line = line << 1;
        }
    }
    return d->width;
}

ssd1306_color_t foreground_color;
ssd1306_color_t background_color;

int ssd1306_draw_string(const ssd1306_t *dev,
                        uint8_t *fb,
                        const font_info_t *font,
                        uint8_t x,
                        uint8_t y,
                        const char *str,
                        ssd1306_color_t foreground,
                        ssd1306_color_t background) {
    uint8_t t = x;
    int err;

    if (font == NULL || str == NULL)
        return 0;

    while (*str) {
        if ((err = ssd1306_draw_char(dev, fb, font, x, y, *str, foreground, background)) < 0)
            return err;
        x += err;
        ++str;
        if (*str)
            x += font->c;
        foreground_color = foreground;
        background_color = background;
    }
    return x - t;
}

int ssd1306_draw_circle(const ssd1306_t *dev,
                        uint8_t *fb,
                        int8_t x0,
                        int8_t y0,
                        uint8_t r,
                        ssd1306_color_t color) {
    int8_t x = r;
    int8_t y = 1;
    int16_t radius_err = 1 - x;
    int err = 0;

    if (r == 0)
        return -EINVAL;

    if ((err = ssd1306_draw_pixel(dev, fb, x0 - r, y0, color)))
        return err;
    if ((err = ssd1306_draw_pixel(dev, fb, x0 + r, y0, color)))
        return err;
    if ((err = ssd1306_draw_pixel(dev, fb, x0, y0 - r, color)))
        return err;
    if ((err = ssd1306_draw_pixel(dev, fb, x0, y0 + r, color)))
        return err;

    while (x >= y) {
        if ((err = ssd1306_draw_pixel(dev, fb, x0 + x, y0 + y, color)))
            return err;
        if ((err = ssd1306_draw_pixel(dev, fb, x0 - x, y0 + y, color)))
            return err;
        if ((err = ssd1306_draw_pixel(dev, fb, x0 + x, y0 - y, color)))
            return err;
        if ((err = ssd1306_draw_pixel(dev, fb, x0 - x, y0 - y, color)))
            return err;
        if (x != y) {
            /* Otherwise the 4 drawings below are the same as above, causing
             * problem when color is INVERT
             */
            if ((err = ssd1306_draw_pixel(dev, fb, x0 + y, y0 + x, color)))
                return err;
            if ((err = ssd1306_draw_pixel(dev, fb, x0 - y, y0 + x, color)))
                return err;
            if ((err = ssd1306_draw_pixel(dev, fb, x0 + y, y0 - x, color)))
                return err;
            if ((err = ssd1306_draw_pixel(dev, fb, x0 - y, y0 - x, color)))
                return err;
        }
        ++y;
        if (radius_err < 0) {
            radius_err += 2 * y + 1;
        } else {
            --x;
            radius_err += 2 * (y - x + 1);
        }
    }
    return 0;
}

int ssd1306_draw_rectangle(const ssd1306_t *dev,
                           uint8_t *fb,
                           int8_t x,
                           int8_t y,
                           uint8_t w,
                           uint8_t h,
                           ssd1306_color_t color) {
    int err = 0;
    if ((err = ssd1306_draw_hline(dev, fb, x, y, w, color)))
        return err;
    if ((err = ssd1306_draw_hline(dev, fb, x, y + h - 1, w, color)))
        return err;
    if ((err = ssd1306_draw_vline(dev, fb, x, y, h, color)))
        return err;
    return ssd1306_draw_vline(dev, fb, x + w - 1, y, h, color);
}