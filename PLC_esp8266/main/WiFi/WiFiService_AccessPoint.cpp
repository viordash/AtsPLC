
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

    const int generation_time_ms = 20000;
    esp_err_t err;
    wifi_config_t wifi_config = {};

    strcpy((char *)wifi_config.ap.ssid, request->Payload.AccessPoint.ssid);
    wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);

    strcpy((char *)wifi_config.ap.password, "AtsPLC_12345678");
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    wifi_config.ap.max_connection = 0;
    // wifi_config.ap.ssid_hidden = true;

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_AccessPoint, "esp_wifi_set_mode err 0x%X", err);
        return;
    }

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_AccessPoint, "esp_wifi_set_config err 0x%X", err);
        return;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_AccessPoint, "esp_wifi_start err 0x%X", err);
        return;
    }

    ESP_LOGI(TAG_WiFiService_AccessPoint, "generating...");
    EventBits_t uxBits = 0;
    do {
        uxBits = xEventGroupWaitBits(event,
                                     STOP_BIT | CANCEL_REQUEST_BIT,
                                     true,
                                     false,
                                     generation_time_ms / portTICK_RATE_MS);
        ESP_LOGI(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", uxBits);

        bool timeout = (uxBits & (STOP_BIT | CANCEL_REQUEST_BIT)) == 0;
        if (timeout) {
            ESP_LOGI(TAG_WiFiService_AccessPoint, "timeout");
            break;
        }
        bool cancel = (uxBits & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_AccessPoint,
                     "Cancel request, ssid:%s",
                     request->Payload.AccessPoint.ssid);
            break;
        }
    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

    esp_wifi_stop();

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    Controller::WakeupProcessTask();

    ESP_LOGW(TAG_WiFiService_AccessPoint, "finish");
}
