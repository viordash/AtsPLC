#include "buttons.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_UP_IO GPIO_NUM_12
#define BUTTON_DOWN_IO GPIO_NUM_13
#define BUTTON_LEFT_IO GPIO_NUM_0
#define BUTTON_RIGHT_IO
#define BUTTON_SELECT_IO GPIO_NUM_14
#define BUTTON_INPUT_PIN_SEL                                                                       \
    ((1ULL << BUTTON_UP_IO) | (1ULL << BUTTON_DOWN_IO) | (1ULL << BUTTON_SELECT_IO))

static const char *TAG = "gpio";

void gpio_init(uint32_t startup_state) {
    gpio_config_t io_conf;
    io_conf.pull_down_en = 0;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = BUTTON_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}
