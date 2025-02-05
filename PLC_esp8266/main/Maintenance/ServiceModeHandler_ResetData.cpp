#include "Display/Common.h"
#include "Display/LogsList.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "Maintenance/ServiceModeHandler.h"
#include "Maintenance/backups_storage.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "redundant_storage.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler_Reset = "ServiceMode.Reset";

void ServiceModeHandler::ResetData(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler_Reset, "execute");

    ResetMode mode = ResetMode::rd_Settings;

    ListBox listBox("Reset data");
    listBox.Insert(0, "Settings");
    listBox.Insert(1, "Ladder program");
    listBox.Insert(2, "Backups");
    listBox.Insert(3, "Factory reset");
    listBox.Select(mode);

    bool success = false;
    bool error = false;
    while (!success && !error) {
        uint8_t *fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        EventBits_t uxBits = xEventGroupWaitBits(gpio_events,
                                                 EXPECTED_BUTTONS,
                                                 true,
                                                 false,
                                                 service_mode_timeout_ms / portTICK_PERIOD_MS);

        bool timeout = (uxBits & EXPECTED_BUTTONS) == 0;
        if (timeout) {
            ESP_LOGI(TAG_ServiceModeHandler_Reset, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler_Reset,
                 "buttons_changed, pressed_button:%u, bits:0x%08X",
                 pressed_button,
                 uxBits);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
                mode = ChangeResetModeToPrev(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
                mode = ChangeResetModeToNext(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
                success = DoResetData(gpio_events, mode);
                error = !success;
                break;
            default:
                break;
        }
    }
    ShowStatus(gpio_events, success, "Reset completed!", "Reset error!");
}

ServiceModeHandler::ResetMode
ServiceModeHandler::ChangeResetModeToPrev(ServiceModeHandler::ResetMode mode) {
    switch (mode) {
        case ResetMode::rd_Settings:
            mode = ResetMode::rd_FactoryReset;
            break;
        case ResetMode::rd_Ladder:
            mode = ResetMode::rd_Settings;
            break;
        case ResetMode::rd_Backups:
            mode = ResetMode::rd_Ladder;
            break;
        case ResetMode::rd_FactoryReset:
            mode = ResetMode::rd_Backups;
            break;
    }
    return mode;
}

ServiceModeHandler::ResetMode
ServiceModeHandler::ChangeResetModeToNext(ServiceModeHandler::ResetMode mode) {
    switch (mode) {
        case ResetMode::rd_Settings:
            mode = ResetMode::rd_Ladder;
            break;
        case ResetMode::rd_Ladder:
            mode = ResetMode::rd_Backups;
            break;
        case ResetMode::rd_Backups:
            mode = ResetMode::rd_FactoryReset;
            break;
        case ResetMode::rd_FactoryReset:
            mode = ResetMode::rd_Settings;
            break;
    }
    return mode;
}

bool ServiceModeHandler::DoResetData(EventGroupHandle_t gpio_events,
                                     ServiceModeHandler::ResetMode mode) {
    ESP_LOGI(TAG_ServiceModeHandler_Reset, "DoResetData, mode:%d", (int)mode);

    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();

    uint8_t *fb = begin_render();
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 1, "Press UP to continue") <= 0);
    end_render(fb);
    xEventGroupWaitBits(gpio_events, BUTTON_UP_IO_OPEN, true, false, portMAX_DELAY);
    xEventGroupClearBits(gpio_events, EXPECTED_BUTTONS);

    return true;
}