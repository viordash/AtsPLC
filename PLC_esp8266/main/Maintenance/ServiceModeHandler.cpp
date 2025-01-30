#include "Maintenance/ServiceModeHandler.h"
#include "Display/Common.h"
#include "Display/LogsList.h"
#include "Display/display.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "settings.h"
#include "smartconfig_service.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler = "service_mode";

#define EXPECTED_BUTTONS                                                                           \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

void ServiceModeHandler::Start(EventGroupHandle_t gpio_events) {
    Mode mode = Mode::sm_SmartConfig;

    while (true) {
        RenderMainMenu(mode);

        EventBits_t uxBits =
            xEventGroupWaitBits(gpio_events, EXPECTED_BUTTONS, true, false, portMAX_DELAY);

        ESP_LOGI(TAG_ServiceModeHandler, "bits:0x%08X", uxBits);

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler, "buttons_changed, pressed_button:%u", pressed_button);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
                mode = ChangeModeToPrev(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
                mode = ChangeModeToNext(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
                Execute(gpio_events, mode);
                return;
            default:
                break;
        }
    }
}

void ServiceModeHandler::RenderMainMenu(Mode mode) {
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
        case Mode::sm_SmartConfig:
            marker_y += height * 1;
            break;
        case Mode::sm_BackupLogic:
            marker_y += height * 2;
            break;
        case Mode::sm_RestoreLogic:
            marker_y += height * 3;
            break;
        case Mode::sm_ResetToDefault:
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
            ESP_LOGE(TAG_ServiceModeHandler, "Mode::sm_BackupLogic not implemented");
            break;
        case Mode::sm_RestoreLogic:
            ESP_LOGE(TAG_ServiceModeHandler, "Mode::sm_RestoreLogic not implemented");
            break;
        case Mode::sm_ResetToDefault:
            ESP_LOGE(TAG_ServiceModeHandler, "Mode::sm_ResetToDefault not implemented");
            break;
    }
}

void ServiceModeHandler::SmartConfig(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler, "exec SmartConfig");
    uint8_t *fb;
    LogsList logs_list("SmartConfig");

    bool runned = true;
    start_smartconfig();
    while (runned) {
        switch (smartconfig_status()) {
            case scs_Start:
                logs_list.Append("Start");
                break;

            case scs_Started:
                logs_list.Append("Started");
                break;

            case scs_Disconnected:
                logs_list.Append("Disconnected 0123456");
                break;

            case scs_GotIP:
                logs_list.Append("Got IP");
                break;

            case scs_ScanDone:
                logs_list.Append("Scanning");
                break;

            case scs_FoundChannel:
                logs_list.Append("Found channel");
                break;

            case scs_GotCreds:
                logs_list.Append("Got credentials");
                break;

            case scs_Completed:
                logs_list.Append("Completed");
                runned = false;
                break;

            case scs_Error:
                logs_list.Append("Error");
                runned = false;
                break;
        }

        fb = begin_render();
        logs_list.Render(fb);
        end_render(fb);
    }

    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();

    const int show_logs_time_ms = 3000;
    xEventGroupWaitBits(gpio_events,
                        EXPECTED_BUTTONS,
                        true,
                        false,
                        show_logs_time_ms / portTICK_PERIOD_MS);

    fb = begin_render();
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 1, "SC completed!") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 2, "Press SELECT to exit") <= 0);
    end_render(fb);

    xEventGroupWaitBits(gpio_events, EXPECTED_BUTTONS, true, false, portMAX_DELAY);
}
