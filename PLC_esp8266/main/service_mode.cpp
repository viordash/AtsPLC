
#include "service_mode.h"
#include "Display/Common.h"
#include "Display/display.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "settings.h"
#include "smartconfig_service.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_service_mode = "service_mode";

enum ServiceMode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

static void render_service_main_menu(ServiceMode mode) {
    char buffer[64];
    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();
    uint8_t *fb = begin_render();

    sprintf(buffer, "v%08X.%s", DEVICE_SETTINGS_VERSION, __TIME__);
    ESP_ERROR_CHECK(draw_text_f5X7(fb, x, y, buffer) <= 0);

    uint8_t marker_x = x;
    uint8_t marker_y = y;
    switch (mode) {
        case ServiceMode::sm_SmartConfig:
            marker_y += height * 1;
            break;
        case ServiceMode::sm_BackupLogic:
            marker_y += height * 2;
            break;
        case ServiceMode::sm_RestoreLogic:
            marker_y += height * 3;
            break;
        case ServiceMode::sm_ResetToDefault:
            marker_y += height * 4;
            break;
    }
    int marker_width = draw_text_f6X12(fb, marker_x, marker_y, "\x01");
    ESP_ERROR_CHECK(marker_width <= 0);
    x += marker_width;

    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 1, "  Smart config") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 2, "  Backup logic") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 3, "  Restore logic") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 4, "  Reset to default") <= 0);

    end_render(fb);
}

static ServiceMode switch_mode_prev(ServiceMode mode) {
    switch (mode) {
        case ServiceMode::sm_SmartConfig:
            mode = ServiceMode::sm_ResetToDefault;
            break;
        case ServiceMode::sm_BackupLogic:
            mode = ServiceMode::sm_SmartConfig;
            break;
        case ServiceMode::sm_RestoreLogic:
            mode = ServiceMode::sm_BackupLogic;
            break;
        case ServiceMode::sm_ResetToDefault:
            mode = ServiceMode::sm_RestoreLogic;
            break;
    }
    return mode;
}

static ServiceMode switch_mode_next(ServiceMode mode) {
    switch (mode) {
        case ServiceMode::sm_SmartConfig:
            mode = ServiceMode::sm_BackupLogic;
            break;
        case ServiceMode::sm_BackupLogic:
            mode = ServiceMode::sm_RestoreLogic;
            break;
        case ServiceMode::sm_RestoreLogic:
            mode = ServiceMode::sm_ResetToDefault;
            break;
        case ServiceMode::sm_ResetToDefault:
            mode = ServiceMode::sm_SmartConfig;
            break;
    }
    return mode;
}

static void execute_service(ServiceMode mode) {
    switch (mode) {
        case ServiceMode::sm_SmartConfig:
            start_smartconfig();
            while (smartconfig_is_runned()) {
                ESP_LOGI(TAG_service_mode, "wait smartconfig...");
                vTaskDelay(3000 / portTICK_PERIOD_MS);
            }
            break;
        case ServiceMode::sm_BackupLogic:
            ESP_LOGE(TAG_service_mode, "ServiceMode::sm_BackupLogic not implemented");
            break;
        case ServiceMode::sm_RestoreLogic:
            ESP_LOGE(TAG_service_mode, "ServiceMode::sm_RestoreLogic not implemented");
            break;
        case ServiceMode::sm_ResetToDefault:
            ESP_LOGE(TAG_service_mode, "ServiceMode::sm_ResetToDefault not implemented");
            break;
    }
}

void run_service_mode(EventGroupHandle_t gpio_events) {
    ServiceMode mode = ServiceMode::sm_SmartConfig;

    while (true) {
        render_service_main_menu(mode);

        EventBits_t uxBits = xEventGroupWaitBits(
            gpio_events,
            BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN
                | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN,
            true,
            false,
            portMAX_DELAY);

        ESP_LOGI(TAG_service_mode, "bits:0x%08X", uxBits);

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_service_mode, "buttons_changed, pressed_button:%u", pressed_button);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
                mode = switch_mode_prev(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
                mode = switch_mode_next(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
                execute_service(mode);
                return;
            default:
                break;
        }
    }
}
