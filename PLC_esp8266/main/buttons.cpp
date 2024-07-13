#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "button.h"
#include "buttons.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio.h"
#include <vector>



static void buttons_task(void *arg);

std::vector<button> buttons{
    { button("button UP", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN) },
    { button("button DOWN", BUTTON_DOWN_IO_CLOSE, BUTTON_DOWN_IO_OPEN) },
    { button("button LEFT", BUTTON_LEFT_IO_CLOSE, BUTTON_LEFT_IO_OPEN) },
    { button("button SELECT", BUTTON_SELECT_IO_CLOSE, BUTTON_SELECT_IO_OPEN) },
};

EventGroupHandle_t buttons_init(EventGroupHandle_t gpio_events) {
    EventGroupHandle_t event = xEventGroupCreate();

    xTaskCreate(buttons_task, "buttons_task", 1024, (void *)gpio_events, 10, NULL);
    return event;
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

        for (auto &button : buttons) {
            switch (button.handle(uxBits)) {
                case button::state::btDown:
                    ESP_LOGI(button.TAG, "process, uxBits:0x%08X btDown", uxBits);
                    break;
                case button::state::btPressed:
                    ESP_LOGI(button.TAG, "process, uxBits:0x%08X btPressed", uxBits);
                    break;
                case button::state::btLongPressed:
                    ESP_LOGI(button.TAG, "process, uxBits:0x%08X btLongPressed", uxBits);
                    break;

                default:
                    break;
            }
        }

        // ESP_LOGI(TAG, "process, uxBits:0x%08X", uxBits);
    }
}
