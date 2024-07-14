#include "display.h"
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
#include <ssd1306/ssd1306.h>

#define SCL_PIN GPIO_NUM_4
#define SDA_PIN GPIO_NUM_5
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

static const char *TAG = "display";

// const font_info_t *font = font_builtin_fonts[FONT_FACE_GLCD5x7];
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

void display_init() {
    // init i2s
    i2c_port_t i2c_master_port = I2C_NUM_0;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.clk_stretch_tick = 300;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    // init ssd1306
    ssd1306_t dev = { .i2c_port = i2c_master_port,
                      .i2c_addr = SSD1306_I2C_ADDR_0,
                      .screen = SSD1306_SCREEN,
                      .width = DISPLAY_WIDTH,
                      .height = DISPLAY_HEIGHT };

    ESP_ERROR_CHECK(ssd1306_init(&dev) != 0 ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(ssd1306_load_frame_buffer(&dev, NULL) != 0 ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(ssd1306_set_whole_display_lighting(&dev, false) != 0 ? ESP_FAIL : ESP_OK);

    ssd1306_draw_string(&dev,
                        buffer,
                        font_builtin_fonts[FONT_FACE_GLCD5x7],
                        0,
                        0,
                        "Hello, esp-rtos!",
                        OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

#define CIRCLE_COUNT_ICON_X 50
#define CIRCLE_COUNT_ICON_Y 30
    for (uint8_t i = 0; i < 10; i++) {
        ssd1306_draw_circle(&dev,
                            buffer,
                            CIRCLE_COUNT_ICON_X,
                            CIRCLE_COUNT_ICON_Y,
                            i,
                            OLED_COLOR_WHITE);
        ssd1306_load_frame_buffer(&dev, buffer);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "init succesfully");
}
