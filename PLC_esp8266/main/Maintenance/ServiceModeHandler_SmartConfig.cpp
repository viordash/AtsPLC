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
    start_smartconfig();
    while (!success && !error) {
        switch (smartconfig_status()) {
            case scs_Start:
                logs_list.Append("Start");
                break;

            case scs_Started:
                logs_list.Append("Started");
                break;

            case scs_Disconnected:
                logs_list.Append("Disconnected");
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
                success = true;
                break;

            case scs_Error:
                logs_list.Append("Error");
                error = true;
                break;
        }

        auto fb = begin_render();
        logs_list.Render(fb);
        end_render(fb);
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
