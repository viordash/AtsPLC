#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "buttons.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio.h"

static const char *TAG = "buttons";

static void buttons_task(void *arg);

void buttons_init(EventGroupHandle_t gpio_events) {
    xTaskCreate(buttons_task, "buttons_task", 1024, (void *)gpio_events, 10, NULL);
}

static void buttons_task(void *arg) {
    EventGroupHandle_t gpio_events = (EventGroupHandle_t)arg;
    while (true) {
        EventBits_t uxBits = xEventGroupWaitBits(
            gpio_events,
            BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN
                | BUTTON_LEFT_IO_CLOSE | BUTTON_LEFT_IO_OPEN | BUTTON_SELECT_IO_CLOSE
                | BUTTON_SELECT_IO_OPEN,
            true,
            false,
            portMAX_DELAY);

        ESP_LOGI(TAG, "process, uxBits:0x%08X", uxBits);
    }
}
