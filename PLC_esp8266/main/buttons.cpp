#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "button.h"
#include "buttons.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include "lassert.h"
#include "sys_gpio.h"
#include <vector>

static const char *TAG_Buttons = "Buttons";

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

const static EventBits_t buttons_close_bits =
    BUTTON_UP_IO_CLOSE | BUTTON_DOWN_IO_CLOSE | BUTTON_SELECT_IO_CLOSE;
const static EventBits_t buttons_open_bits =
    BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_OPEN | BUTTON_SELECT_IO_OPEN;

const static uint32_t min_pressed_time_ms = 20;
static EventBits_t deffered_button_bits = 0;

static ButtonsPressType process_buttons(EventBits_t uxBits) {
    for (auto &button : buttons.buttons) {
        switch (button.handle(uxBits)) {
            case button::state::btDown:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btDown", (unsigned int)uxBits);
                break;

            case button::state::btPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btPressed", (unsigned int)uxBits);
                return button.pressed_type;

            case button::state::btLongPressed:
                ESP_LOGD(button.TAG, "process, uxBits:0x%08X btLongPressed", (unsigned int)uxBits);
                return button.long_pressed_type;

            default:
                break;
        }
    }
    return ButtonsPressType::NOTHING_PRESSED;
}

ButtonsPressType handle_buttons(EventBits_t uxBits, ProcessWakeupService *wakeup_service) {
    ASSERT(wakeup_service != NULL);

    EventBits_t button_close_bits = (uxBits & buttons_close_bits);
    EventBits_t button_open_bits = (uxBits & buttons_open_bits);

    if (button_close_bits != 0) {
        ESP_LOGD(TAG_Buttons, "button_close, bits:0x%08X", (unsigned int)uxBits);

        wakeup_service->RemoveRequest((void *)&handle_buttons);
        ASSERT(wakeup_service->Request((void *)&handle_buttons,
                                       min_pressed_time_ms,
                                       ProcessWakeupRequestPriority::pwrp_Idle));
        deffered_button_bits = 0;
        process_buttons(button_close_bits);
    }

    if (button_open_bits != 0) {
        deffered_button_bits = button_open_bits;
        ESP_LOGD(TAG_Buttons, "button_open, bits:0x%08X", (unsigned int)deffered_button_bits);

        wakeup_service->RemoveRequest((void *)&handle_buttons);
        wakeup_service->Request((void *)&handle_buttons,
                                min_pressed_time_ms,
                                ProcessWakeupRequestPriority::pwrp_Idle);
        return ButtonsPressType::NOTHING_PRESSED;
    }

    if (deffered_button_bits == 0 || wakeup_service->Contains((void *)&handle_buttons)) {
        return ButtonsPressType::NOTHING_PRESSED;
    }

    ButtonsPressType pressed_button = process_buttons(deffered_button_bits);
    ESP_LOGD(TAG_Buttons,
             "buttons_changed, bits:0x%08X, pressed_button:%u",
             (unsigned int)deffered_button_bits,
             pressed_button);

    deffered_button_bits = 0;

    return pressed_button;
}
