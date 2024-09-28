
#include "button.h"
#include "esp_timer.h"

button::button(const char *tag,
               EventBits_t close_bit,
               EventBits_t open_bit,
               ButtonsPressType pressed_type,
               ButtonsPressType long_pressed_type) {
    this->TAG = tag;
    this->close_bit = close_bit;
    this->open_bit = open_bit;
    this->pressed_type = pressed_type;
    this->long_pressed_type = long_pressed_type;
    down = false;
    down_time = 0;
}

button::state button::handle(EventBits_t bits) {
    if (!down && (bits & close_bit) != 0) {
        down_time = (uint64_t)esp_timer_get_time();
        down = true;
        return button::state::btDown;
    }

    if (down && (bits & open_bit) != 0) {
        down = false;
        uint64_t now_time = (uint64_t)esp_timer_get_time();
        uint64_t duration_us =
            now_time >= down_time ? now_time - down_time : (UINT64_MAX - down_time) + now_time;
        uint32_t duration_ms = duration_us / 1000;
        if (duration_ms >= button::long_pressed_time_ms) {
            return button::state::btLongPressed;
        }
        if (duration_ms >= button::pressed_time_ms) {
            return button::state::btPressed;
        }
        return button::state::btShortPressed;
    }
    return button::state::btNone;
}
