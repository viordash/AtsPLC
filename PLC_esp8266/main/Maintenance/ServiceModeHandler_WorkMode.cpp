#include "Display/Common.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "Maintenance/ServiceModeHandler.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "hotreload_service.h"
#include "lassert.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler_WorkMode = "ServiceMode.WorkMode";

const char *ServiceModeHandler::work_mode_names[] = { "Stop", "Run", "Debug" };

void ServiceModeHandler::ChangeWorkMode(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler_WorkMode, "execute");

    Controller::GetLadder().Load();

    bool enable_debug;
    SAFETY_HOTRELOAD({ enable_debug = hotreload->enable_debug; });

    EffectiveWorkMode mode =
        Convert2EffectiveWorkMode(Controller::GetLadder().GetWorkMode(), enable_debug);

    const size_t modes_count = sizeof(work_mode_names) / sizeof(work_mode_names[0]);
    char title[32];
    ASSERT((size_t)mode < modes_count);
    snprintf(title, sizeof(title), "WorkMode(%s)", work_mode_names[mode]);

    ListBox listBox(title);
    for (size_t i = 0; i < modes_count; i++) {
        listBox.Insert(i, work_mode_names[i]);
    }
    listBox.Select(mode);

    bool success = false;
    bool error = false;
    while (!success && !error) {
        auto fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        EventBits_t uxBits = xEventGroupWaitBits(gpio_events,
                                                 EXPECTED_BUTTONS,
                                                 true,
                                                 false,
                                                 service_mode_timeout_ms / portTICK_PERIOD_MS);

        bool timeout = (uxBits & EXPECTED_BUTTONS) == 0;
        if (timeout) {
            ESP_LOGI(TAG_ServiceModeHandler_WorkMode, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler_WorkMode,
                 "buttons_changed, pressed_button:%u, bits:0x%08X",
                 (unsigned int)pressed_button,
                 (unsigned int)uxBits);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
            case ButtonsPressType::UP_LONG_PRESSED:
                mode = ChangeWorkModeToPrev(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::DOWN_PRESSED:
            case ButtonsPressType::DOWN_LONG_PRESSED:
                mode = ChangeWorkModeToNext(mode);
                listBox.Select(mode);
                break;
            case ButtonsPressType::SELECT_PRESSED:
            case ButtonsPressType::SELECT_LONG_PRESSED:
                success = DoChangeWorkMode(gpio_events, mode);
                error = !success;
                break;
            default:
                break;
        }
    }
    ShowStatus(gpio_events, success, "Completed!", "Canceled!");
    esp_restart();
}

EffectiveWorkMode ServiceModeHandler::ChangeWorkModeToPrev(EffectiveWorkMode mode) {
    switch (mode) {
        case EffectiveWorkMode::ewm_Stop:
            mode = EffectiveWorkMode::ewm_Debug;
            break;
        case EffectiveWorkMode::ewm_Run:
            mode = EffectiveWorkMode::ewm_Stop;
            break;
        case EffectiveWorkMode::ewm_Debug:
            mode = EffectiveWorkMode::ewm_Run;
            break;
    }
    return mode;
}

EffectiveWorkMode ServiceModeHandler::ChangeWorkModeToNext(EffectiveWorkMode mode) {
    switch (mode) {
        case EffectiveWorkMode::ewm_Stop:
            mode = EffectiveWorkMode::ewm_Run;
            break;
        case EffectiveWorkMode::ewm_Run:
            mode = EffectiveWorkMode::ewm_Debug;
            break;
        case EffectiveWorkMode::ewm_Debug:
            mode = EffectiveWorkMode::ewm_Stop;
            break;
    }
    return mode;
}

bool ServiceModeHandler::DoChangeWorkMode(EventGroupHandle_t gpio_events, EffectiveWorkMode mode) {
    ESP_LOGI(TAG_ServiceModeHandler_WorkMode, "DoChangeWorkMode, mode:%d", (int)mode);

    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();

    auto fb = begin_render();
    ASSERT(draw_text_f6X12(fb, x, y + height * 1, "WorkMode:") > 0);
    ASSERT(draw_text_f6X12(fb, x, y + height * 2, work_mode_names[mode]) > 0);
    ASSERT(draw_text_f6X12(fb, x, y + height * 3, "Press UP to continue") > 0);
    fb->has_changes = true;
    end_render(fb);

    EventBits_t uxBits =
        xEventGroupWaitBits(gpio_events,
                            BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_OPEN | BUTTON_SELECT_IO_OPEN,
                            true,
                            false,
                            portMAX_DELAY);

    ESP_LOGI(TAG_ServiceModeHandler_WorkMode, "bits:0x%08X", (unsigned int)uxBits);

    bool button_up_released = (uxBits & (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN)) != 0;
    if (!button_up_released) {
        ESP_LOGI(TAG_ServiceModeHandler_WorkMode, "Canceled");
        return false;
    }

    Controller::GetLadder().ChangeWorkMode(Convert2WorkMode(mode), Convert2EnableDebug(mode));
    return true;
}
