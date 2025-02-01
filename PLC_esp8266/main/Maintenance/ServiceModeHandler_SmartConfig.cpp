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
    uint8_t *fb;
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
    ESP_ERROR_CHECK(draw_text_f6X12(fb,
                                    x,
                                    y + height * 1,
                                    success //
                                        ? "SC completed!"
                                        : "SC error!")
                    <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 2, "Press SELECT to exit") <= 0);
    end_render(fb);

    xEventGroupWaitBits(gpio_events, EXPECTED_BUTTONS, true, false, portMAX_DELAY);
}
