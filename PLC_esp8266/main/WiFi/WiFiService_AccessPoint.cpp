
#include "LogicProgram/Controller.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

#define INFINITY_CONNECT_RETRY -1

static const char *TAG_WiFiService_AccessPoint = "WiFiService.AccessPoint";
extern device_settings settings;

void WiFiService::AccessPointTask(RequestItem *request) {
    ESP_LOGW(TAG_WiFiService_AccessPoint, "start, ssid:%s", request->Payload.AccessPoint.ssid);

    EventBits_t uxBits = 0;
    do {
        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     /*portMAX_DELAY*/ 10000 / portTICK_RATE_MS);
        ESP_LOGI(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", uxBits);

        bool timeout = (uxBits & (STOP_BIT | CANCEL_REQUEST_BIT)) == 0;
        if (timeout) {
            ESP_LOGI(TAG_WiFiService_AccessPoint, "found");
            break;
        }
        bool cancel = (uxBits & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_AccessPoint, "Cancel request, ssid:%s", request->Payload.AccessPoint.ssid);
            break;
        }
    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    Controller::WakeupProcessTask();

    ESP_LOGW(TAG_WiFiService_AccessPoint, "finish");
}
