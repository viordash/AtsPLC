#include "gpio.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define GPIO_OUTPUT_IO_0 2
#define GPIO_OUTPUT_IO_1 15
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0 0
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0))

static const char *TAG = "gpio";

void gpio_init(uint32_t startup_state) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    set_digital_value(OUTPUT_0, startup_state & OUTPUT_0);
    set_digital_value(OUTPUT_1, startup_state & OUTPUT_1);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8; // ADC sample collection clock = 80MHz/clk_div = 10MHz
    ESP_ERROR_CHECK(adc_init(&adc_config));
}

bool get_digital_value(gpio_output gpio) {
    switch (gpio) {
        case OUTPUT_0:
            return gpio_get_level(GPIO_OUTPUT_IO_0);
        case OUTPUT_1:
            return gpio_get_level(GPIO_OUTPUT_IO_1);
    }
    ESP_LOGE(TAG, "get_digital_value, err:0x%X\r\n", ESP_ERR_NOT_FOUND);
    return false;
}

void set_digital_value(gpio_output gpio, bool value) {
    esp_err_t err = ESP_ERR_NOT_FOUND;
    switch (gpio) {
        case OUTPUT_0:
            err = gpio_set_level(GPIO_OUTPUT_IO_0, value);
            break;
        case OUTPUT_1:
            err = gpio_set_level(GPIO_OUTPUT_IO_1, value);
            break;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "set_digital_value, err:0x%X\r\n", err);
    }
}

uint16_t get_analog_value() {
    uint16_t adc = 0xFFFF;
    esp_err_t err = adc_read(&adc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "get_analog_value, err:0x%X\r\n", err);
    }
    return adc;
}