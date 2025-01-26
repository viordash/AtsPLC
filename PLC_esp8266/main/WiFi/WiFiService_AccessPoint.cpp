
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

static const char *TAG_WiFiService_AccessPoint = "WiFiService.AccessPoint";
extern device_settings settings;

void WiFiService::AccessPointTask(RequestItem *request) {
    ESP_LOGD(TAG_WiFiService_AccessPoint, "start, ssid:%s", request->Payload.AccessPoint.ssid);

    wifi_access_point_settings access_point_settings;
    SAFETY_SETTINGS({ access_point_settings = settings.wifi_access_point; });

    esp_err_t err;
    wifi_config_t wifi_config = {};

    strcpy((char *)wifi_config.ap.ssid, request->Payload.AccessPoint.ssid);
    wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);

    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    wifi_config.ap.max_connection = 0;
    wifi_config.ap.ssid_hidden = access_point_settings.ssid_hidden;

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

    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "generating ssid:'%s'...",
             request->Payload.AccessPoint.ssid);

    bool cancel = false;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        bool notify_wait_timeout =
            xTaskNotifyWait(0,
                            CANCEL_REQUEST_BIT,
                            &ulNotifiedValue,
                            access_point_settings.generation_time_ms / portTICK_RATE_MS)
            == pdFALSE;

        ESP_LOGD(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", ulNotifiedValue);

        if (notify_wait_timeout) {
            ESP_LOGD(TAG_WiFiService_AccessPoint, "timeout");
            break;
        }

        bool to_stop = (ulNotifiedValue & STOP_BIT) != 0;
        if (to_stop) {
            break;
        }

        cancel = (ulNotifiedValue & CANCEL_REQUEST_BIT) != 0 && !requests.Contains(request);
        if (cancel) {
            ESP_LOGI(TAG_WiFiService_AccessPoint,
                     "Cancel request, ssid:%s",
                     request->Payload.AccessPoint.ssid);
            break;
        }
    }

    esp_wifi_stop();

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    if (!cancel) {
        requests.AccessPoint(request->Payload.AccessPoint.ssid);
    }

    Controller::WakeupProcessTask();

    ESP_LOGD(TAG_WiFiService_AccessPoint, "finish");
}
