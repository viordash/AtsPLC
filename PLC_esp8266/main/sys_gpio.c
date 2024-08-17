#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "driver/gpio.h"
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

static const char *TAG = "gpio";

static struct {
    EventGroupHandle_t event;
} gpio;

static void outputs_init(uint32_t startup_state) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    set_digital_value(OUTPUT_0, startup_state & OUTPUT_0);
    set_digital_value(OUTPUT_1, startup_state & OUTPUT_1);
}

static void BUTTON_UP_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_UP_IO) == 0) {
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
static void BUTTON_DOWN_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_DOWN_IO) == 0) {
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
static void BUTTON_RIGHT_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_RIGHT_IO) == 0) {
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
static void BUTTON_SELECT_IO_isr_handler(void *arg) {
    (void)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;
    if (gpio_get_level(BUTTON_SELECT_IO) == 0) {
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

EventGroupHandle_t gpio_init(uint32_t startup_state) {
    outputs_init(startup_state);

    gpio.event = xEventGroupCreate();

    inputs_init();
    analog_init();
    return gpio.event;
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
            err = gpio_set_level(GPIO_OUTPUT_IO_0, value ? 1 : 0);
            break;
        case OUTPUT_1:
            err = gpio_set_level(GPIO_OUTPUT_IO_1, value ? 1 : 0);
            gpio_set_level(GPIO_OUTPUT_LED, value ? 1 : 0);
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

bool get_digital_input_value() {
    const int input_normaly_close_value = 0;
    bool input_closed = gpio_get_level(INPUT_1_IO) == input_normaly_close_value;
    return input_closed;
}

bool select_button_pressed() {
    bool state = gpio_get_level(BUTTON_SELECT_IO) == 0;
    ESP_LOGI(TAG, "select_button_pressed, state:%d", state);
    return state;
}