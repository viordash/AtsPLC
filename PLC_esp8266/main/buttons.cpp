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
                 TButtons::UP_PRESSED,
                 TButtons::UP_LONG_PRESSED) },
        { button("button DOWN",
                 BUTTON_DOWN_IO_CLOSE,
                 BUTTON_DOWN_IO_OPEN,
                 TButtons::DOWN_PRESSED,
                 TButtons::DOWN_LONG_PRESSED) },
        { button("button RIGHT",
                 BUTTON_RIGHT_IO_CLOSE,
                 BUTTON_RIGHT_IO_OPEN,
                 TButtons::RIGHT_PRESSED,
                 TButtons::RIGHT_LONG_PRESSED) },
        { button("button SELECT",
                 BUTTON_SELECT_IO_CLOSE,
                 BUTTON_SELECT_IO_OPEN,
                 TButtons::SELECT_PRESSED,
                 TButtons::SELECT_LONG_PRESSED) },
    };
} buttons;

TButtons handle_buttons(EventBits_t uxBits) {
    for (auto &button : buttons.buttons) {
        switch (button.handle(uxBits)) {
            case button::state::btDown:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btDown", uxBits);
                break;
            case button::state::btShortPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btShortPressed", uxBits);
                break;
            case button::state::btPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btPressed", uxBits);
                return button.pressed_type;
                break;
            case button::state::btLongPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btLongPressed", uxBits);
                return button.long_pressed_type;
                break;

            default:
                break;
        }
    }
    return TButtons::NOTHING_PRESSED;
}
