#include "Maintenance/ServiceModeHandler.h"
#include "Display/Common.h"
#include "Display/ListBox.h"
#include "Display/display.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler = "ServiceMode";

#define EXPECTED_BUTTONS                                                                           \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

void ServiceModeHandler::Start(EventGroupHandle_t gpio_events) {
    char buffer[64];
    Mode mode = Mode::sm_SmartConfig;

    sprintf(buffer, "v%08X.%u", DEVICE_SETTINGS_VERSION, (unsigned)BUILD_NUMBER);
    ListBox listBox(buffer);
    listBox.Insert(0, "Smart config");
    listBox.Insert(1, "Backup logic");
    listBox.Insert(2, "Restore logic");
    listBox.Insert(3, "Reset to default");
    listBox.Select(mode);

    while (true) {
        auto fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        EventBits_t uxBits = xEventGroupWaitBits(gpio_events,
                                                 EXPECTED_BUTTONS,
                                                 true,
                                                 false,
                                                 service_mode_timeout_ms / portTICK_PERIOD_MS);

        ESP_LOGD(TAG_ServiceModeHandler, "bits:0x%08X", (unsigned int)uxBits);

        bool timeout = (uxBits & EXPECTED_BUTTONS) == 0;
        if (timeout) {
            ESP_LOGI(TAG_ServiceModeHandler, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler, "buttons_changed, pressed_button:%u", pressed_button);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
            case ButtonsPressType::UP_LONG_PRESSED:
                mode = ChangeModeToPrev(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
            case ButtonsPressType::DOWN_LONG_PRESSED:
                mode = ChangeModeToNext(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
            case ButtonsPressType::SELECT_LONG_PRESSED:
                Execute(gpio_events, mode);
                return;
            default:
                break;
        }
    }
}

ServiceModeHandler::Mode ServiceModeHandler::ChangeModeToPrev(ServiceModeHandler::Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            mode = Mode::sm_ResetToDefault;
            break;
        case Mode::sm_BackupLogic:
            mode = Mode::sm_SmartConfig;
            break;
        case Mode::sm_RestoreLogic:
            mode = Mode::sm_BackupLogic;
            break;
        case Mode::sm_ResetToDefault:
            mode = Mode::sm_RestoreLogic;
            break;
    }
    return mode;
}

ServiceModeHandler::Mode ServiceModeHandler::ChangeModeToNext(ServiceModeHandler::Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            mode = Mode::sm_BackupLogic;
            break;
        case Mode::sm_BackupLogic:
            mode = Mode::sm_RestoreLogic;
            break;
        case Mode::sm_RestoreLogic:
            mode = Mode::sm_ResetToDefault;
            break;
        case Mode::sm_ResetToDefault:
            mode = Mode::sm_SmartConfig;
            break;
    }
    return mode;
}

void ServiceModeHandler::Execute(EventGroupHandle_t gpio_events, Mode mode) {
    switch (mode) {
        case Mode::sm_SmartConfig:
            SmartConfig(gpio_events);
            break;
        case Mode::sm_BackupLogic:
            Backup(gpio_events);
            break;
        case Mode::sm_RestoreLogic:
            Restore(gpio_events);
            break;
        case Mode::sm_ResetToDefault:
            ResetData(gpio_events);
            break;
    }
}

void ServiceModeHandler::ShowStatus(EventGroupHandle_t gpio_events,
                                    bool success,
                                    const char *success_message,
                                    const char *error_message) {
    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();

    xEventGroupClearBits(gpio_events, EXPECTED_BUTTONS);
    auto fb = begin_render();
    ASSERT(draw_text_f6X12(fb,
                           x,
                           y + height * 1,
                           success //
                               ? success_message
                               : error_message)
           > 0);
    ASSERT(draw_text_f6X12(fb, x, y + height * 2, "Press SELECT to exit") > 0);
    fb->has_changes = true;
    end_render(fb);

    xEventGroupWaitBits(gpio_events, BUTTON_SELECT_IO_OPEN, true, false, portMAX_DELAY);
}