#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "sys_gpio.h"

#define GPIO_OUTPUT_IO_0 GPIO_NUM_2
#define GPIO_OUTPUT_IO_1 GPIO_NUM_15
#define GPIO_OUTPUT_LED GPIO_NUM_16
#define GPIO_OUTPUT_PIN_SEL                                                                        \
    ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_LED))

#define BUTTON_UP_IO GPIO_NUM_13
#define BUTTON_DOWN_IO GPIO_NUM_12
#define BUTTON_LEFT_IO
#define BUTTON_RIGHT_IO GPIO_NUM_0
#define BUTTON_SELECT_IO GPIO_NUM_14

#define INPUT_1_IO GPIO_NUM_0

#define GPIO_INPUT_PIN_SEL                                                                         \
    ((1ULL << BUTTON_UP_IO) | (1ULL << BUTTON_DOWN_IO) | (1ULL << BUTTON_RIGHT_IO)                 \
     | (1ULL << BUTTON_SELECT_IO))

#define GPIO_ACTIVE 0
#define GPIO_PASSIVE 1
#define INPUT_NC_VALUE 0
#define INPUT_NO_VALUE 1

static const char *TAG_gpio = "gpio";

static struct {
    EventGroupHandle_t event;
} gpio;

static void outputs_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    set_digital_value(OUTPUT_0, false);
    set_digital_value(OUTPUT_1, false);
}

static IRAM_ATTR void BUTTON_UP_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_UP_IO) == INPUT_NC_VALUE) {
        xResult =
            xEventGroupSetBitsFromISR(gpio.event, BUTTON_UP_IO_CLOSE, &xHigherPriorityTaskWoken);
    } else {
        xResult =
            xEventGroupSetBitsFromISR(gpio.event, BUTTON_UP_IO_OPEN, &xHigherPriorityTaskWoken);
    }
    if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
}
static IRAM_ATTR void BUTTON_DOWN_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_DOWN_IO) == INPUT_NC_VALUE) {
        xResult =
            xEventGroupSetBitsFromISR(gpio.event, BUTTON_DOWN_IO_CLOSE, &xHigherPriorityTaskWoken);
    } else {
        xResult =
            xEventGroupSetBitsFromISR(gpio.event, BUTTON_DOWN_IO_OPEN, &xHigherPriorityTaskWoken);
    }
    if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
}
static IRAM_ATTR void BUTTON_RIGHT_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_RIGHT_IO) == INPUT_NC_VALUE) {
        xResult = xEventGroupSetBitsFromISR(gpio.event,
                                            BUTTON_RIGHT_IO_CLOSE | INPUT_1_IO_CLOSE,
                                            &xHigherPriorityTaskWoken);
    } else {
        xResult = xEventGroupSetBitsFromISR(gpio.event,
                                            BUTTON_RIGHT_IO_OPEN | INPUT_1_IO_OPEN,
                                            &xHigherPriorityTaskWoken);
    }
    if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
}
static IRAM_ATTR void BUTTON_SELECT_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_SELECT_IO) == INPUT_NC_VALUE) {
        xResult = xEventGroupSetBitsFromISR(gpio.event,
                                            BUTTON_SELECT_IO_CLOSE,
                                            &xHigherPriorityTaskWoken);
    } else {
        xResult =
            xEventGroupSetBitsFromISR(gpio.event, BUTTON_SELECT_IO_OPEN, &xHigherPriorityTaskWoken);
    }
    if (xResult == pdPASS && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR();
    }
}

static void inputs_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_UP_IO, BUTTON_UP_IO_isr_handler, NULL);
    gpio_isr_handler_add(BUTTON_DOWN_IO, BUTTON_DOWN_IO_isr_handler, NULL);
    gpio_isr_handler_add(BUTTON_RIGHT_IO, BUTTON_RIGHT_IO_isr_handler, NULL);
    gpio_isr_handler_add(BUTTON_SELECT_IO, BUTTON_SELECT_IO_isr_handler, NULL);
}

static void analog_init() {
    adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8; // ADC sample collection clock = 80MHz/clk_div = 10MHz
    ESP_ERROR_CHECK(adc_init(&adc_config));
}

EventGroupHandle_t gpio_init() {
    outputs_init();

    gpio.event = xEventGroupCreate();

    inputs_init();
    analog_init();
    return gpio.event;
}

bool get_digital_value(gpio_output gpio) {
    switch (gpio) {
        case OUTPUT_0:
            return gpio_get_level(GPIO_OUTPUT_IO_0) == GPIO_ACTIVE;
        case OUTPUT_1:
            return gpio_get_level(GPIO_OUTPUT_IO_1) == GPIO_ACTIVE;
    }
    ESP_LOGE(TAG_gpio, "get_digital_value, err:0x%X", ESP_ERR_NOT_FOUND);
    return false;
}

void set_digital_value(gpio_output gpio, bool value) {
    esp_err_t err = ESP_ERR_NOT_FOUND;
    int gpio_val = value ? GPIO_ACTIVE : GPIO_PASSIVE;
    switch (gpio) {
        case OUTPUT_0:
            err = gpio_set_level(GPIO_OUTPUT_IO_0, gpio_val);
            break;
        case OUTPUT_1:
            err = gpio_set_level(GPIO_OUTPUT_IO_1, gpio_val);
            gpio_set_level(GPIO_OUTPUT_LED, gpio_val);
            break;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG_gpio, "set_digital_value, err:0x%X", err);
    }
}

uint16_t get_analog_value() {
    uint16_t adc = 0x0;
    esp_err_t err = adc_read(&adc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_gpio, "get_analog_value, err:0x%X", err);
    }
    return adc;
}

bool get_digital_input_value() {
    bool input_closed = gpio_get_level(INPUT_1_IO) == INPUT_NC_VALUE;
    return input_closed;
}

bool up_button_pressed() {
    bool state = gpio_get_level(BUTTON_UP_IO) == 0;
    ESP_LOGD(TAG_gpio, "select_button_pressed, state:%d", state);
    return state;
}