#include "Display/Common.h"
#include "Display/LogsList.h"
#include "Display/display.h"
#include "Maintenance/ServiceModeHandler.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "smartconfig_service.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler_SmartConfig = "ServiceMode.SC";

void ServiceModeHandler::SmartConfig(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler_SmartConfig, "execute");
    LogsList logs_list("SmartConfig");

    bool success = false;
    bool error = false;

    SmartConfigProgress(logs_list, "Prepare");

    start_smartconfig();
    while (!success && !error) {
        switch (smartconfig_status()) {
            case scs_Start:
                SmartConfigProgress(logs_list, "Start");
                break;

            case scs_Started:
                SmartConfigProgress(logs_list, "Started");
                break;

            case scs_Disconnected:
                SmartConfigProgress(logs_list, "Disconnected");
                break;

            case scs_GotIP:
                SmartConfigProgress(logs_list, "Got IP");
                break;

            case scs_ScanDone:
                SmartConfigProgress(logs_list, "Scanning");
                break;

            case scs_FoundChannel:
                SmartConfigProgress(logs_list, "Found channel");
                break;

            case scs_GotCreds:
                SmartConfigProgress(logs_list, "Got credentials");
                break;

            case scs_Completed:
                SmartConfigProgress(logs_list, "Completed");
                success = true;
                break;

            case scs_Error:
                SmartConfigProgress(logs_list, "Error");
                error = true;
                break;
        }
    }

    const int show_logs_time_ms = 3000;
    xEventGroupWaitBits(gpio_events,
                        EXPECTED_BUTTONS,
                        true,
                        false,
                        show_logs_time_ms / portTICK_PERIOD_MS);

    ShowStatus(gpio_events, success, "SC completed!", "SC error!");
    finish_smartconfig();
}

void ServiceModeHandler::SmartConfigProgress(LogsList &logs_list, const char *message) {
    logs_list.Append(message);
    auto fb = begin_render();
    logs_list.Render(fb);
    end_render(fb);
}
