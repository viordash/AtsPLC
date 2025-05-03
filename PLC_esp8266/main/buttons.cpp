#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "button.h"
#include "buttons.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include "sys_gpio.h"
#include <vector>

static struct {
    std::vector<button> buttons{
        { button("button UP",
                 BUTTON_UP_IO_CLOSE,
                 BUTTON_UP_IO_OPEN,
                 ButtonsPressType::UP_PRESSED,
                 ButtonsPressType::UP_LONG_PRESSED) },
        { button("button DOWN",
                 BUTTON_DOWN_IO_CLOSE,
                 BUTTON_DOWN_IO_OPEN,
                 ButtonsPressType::DOWN_PRESSED,
                 ButtonsPressType::DOWN_LONG_PRESSED) },
        { button("button SELECT",
                 BUTTON_SELECT_IO_CLOSE,
                 BUTTON_SELECT_IO_OPEN,
                 ButtonsPressType::SELECT_PRESSED,
                 ButtonsPressType::SELECT_LONG_PRESSED) },
    };
} buttons;

ButtonsPressType handle_buttons(EventBits_t uxBits) {
    for (auto &button : buttons.buttons) {
        switch (button.handle(uxBits)) {
            case button::state::btDown:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btDown", (unsigned int)uxBits);
                break;
            case button::state::btShortPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btShortPressed", (unsigned int)uxBits);
                break;
            case button::state::btPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btPressed", (unsigned int)uxBits);
                return button.pressed_type;
                break;
            case button::state::btLongPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btLongPressed", (unsigned int)uxBits);
                return button.long_pressed_type;
                break;

            default:
                break;
        }
    }
    return ButtonsPressType::NOTHING_PRESSED;
}
