#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "button.h"

button::button(const char *tag, EventBits_t close_bit, EventBits_t open_bit) {
    this->TAG = tag;
    this->close_bit = close_bit;
    this->open_bit = open_bit;
    down = false;
}

button::state button::handle(EventBits_t bits) {
    if (!down && (bits & close_bit) != 0) {
        down_time = xTaskGetTickCount();
        down = true;
        return button::state::btDown;
    }

    if (down && (bits & open_bit) != 0) {
        down = false;
        TickType_t now_time = xTaskGetTickCount();
        uint32_t duration =
            now_time >= down_time ? now_time - down_time : (UINT32_MAX - down_time) + now_time;
        if (duration >= button::long_pressed_time) {
            return button::state::btLongPressed;
        }
        if (duration >= button::pressed_time) {
            return button::state::btPressed;
        }
        return button::state::btShortPressed;
    }
    return button::state::btNone;
}
