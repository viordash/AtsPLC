#include "CppUTestExt/MockSupport_c.h"
#include "driver/adc.h"
#include "driver/gpio.h"

esp_err_t gpio_config(const gpio_config_t *config) {
    return mock_c()
        ->actualCall("gpio_config")
        ->withConstPointerParameters("config", config)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t adc_init(adc_config_t *config) {
    return mock_c()
        ->actualCall("adc_init")
        ->withConstPointerParameters("config", config)
        ->returnIntValueOrDefault(ESP_OK);
}

int gpio_get_level(gpio_num_t gpio_num) {
    return mock_c()
        ->actualCall("gpio_get_level")
        ->withIntParameters("gpio_num", gpio_num)
        ->returnIntValueOrDefault(-1);
}

esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) {
    switch (gpio_num) {
        case GPIO_NUM_2:
            return mock_c()
                ->actualCall("gpio_set_level_0")
                ->withIntParameters("gpio_num", gpio_num)
                ->withUnsignedIntParameters("level", level)
                ->returnIntValueOrDefault(ESP_OK);

        case GPIO_NUM_15:
            return mock_c()
                ->actualCall("gpio_set_level_1")
                ->withIntParameters("gpio_num", gpio_num)
                ->withUnsignedIntParameters("level", level)
                ->returnIntValueOrDefault(ESP_OK);
        default:
            return mock_c()
                ->actualCall("gpio_set_level")
                ->withIntParameters("gpio_num", gpio_num)
                ->withUnsignedIntParameters("level", level)
                ->returnIntValueOrDefault(ESP_OK);
    }
}

esp_err_t adc_read(uint16_t *data) {
    return mock_c()
        ->actualCall("adc_read")
        ->withPointerParameters("data", data)
        ->returnIntValueOrDefault(ESP_OK);
}