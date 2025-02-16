
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
extern CurrentSettings::device_settings settings;

void WiFiService::AccessPointTask(RequestItem *request) {

    CurrentSettings::wifi_access_point_settings access_point_settings;
    SAFETY_SETTINGS({ access_point_settings = settings.wifi_access_point; });

    esp_err_t err;
    wifi_config_t wifi_config = {};

    strcpy((char *)wifi_config.ap.ssid, request->Payload.AccessPoint.ssid);
    wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);

    bool secure_client = request->Payload.AccessPoint.password != NULL
                      && strlen(request->Payload.AccessPoint.password) > 0;

    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "start, ssid:'%s', password:'%s', mac:'%s'",
             request->Payload.AccessPoint.ssid,
             secure_client ? request->Payload.AccessPoint.password : "",
             secure_client ? request->Payload.AccessPoint.mac : "");

    wifi_config.ap.authmode = secure_client ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    if (secure_client) {
        strcpy((char *)wifi_config.ap.password, request->Payload.AccessPoint.password);
    }
    wifi_config.ap.max_connection = secure_client ? 4 : 0;
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

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_AP_STACONNECTED,
                                               &ap_wifi_event_handler,
                                               this));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "generating ssid:'%s'...",
             request->Payload.AccessPoint.ssid);

    bool cancel = false;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        bool notify_wait_timeout =
            xTaskNotifyWait(0,
                            CANCEL_REQUEST_BIT | CONNECTED_BIT,
                            &ulNotifiedValue,
                            access_point_settings.generation_time_ms / portTICK_RATE_MS)
            == pdFALSE;

        ESP_LOGD(TAG_WiFiService_AccessPoint, "process, uxBits:0x%08X", ulNotifiedValue);

        if (notify_wait_timeout && requests.OneMoreInQueue()) {
            ESP_LOGI(TAG_WiFiService_AccessPoint, "Stop AP due to new request");
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

        bool connected = (ulNotifiedValue & CONNECTED_BIT) != 0;
        if (connected) {
            wifi_sta_list_t sta_list;
            if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK) {
                ESP_LOGI(TAG_WiFiService_AccessPoint, "devices connected: %d", sta_list.num);
                for (int i = 0; i < sta_list.num; i++) {
                    ESP_LOGI(TAG_WiFiService_AccessPoint,
                             "%d. " MACSTR,
                             i,
                             MAC2STR(sta_list.sta[i].mac));
                }
            }
        }
    }

    esp_wifi_stop();

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                                                 WIFI_EVENT_AP_STACONNECTED,
                                                 &ap_wifi_event_handler));

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    if (!cancel) {
        requests.AccessPoint(request->Payload.AccessPoint.ssid,
                             request->Payload.AccessPoint.password,
                             request->Payload.AccessPoint.mac);
    }

    Controller::WakeupProcessTask();

    ESP_LOGD(TAG_WiFiService_AccessPoint, "finish");
}

void WiFiService::ap_wifi_event_handler(void *arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void *event_data) {
    (void)event_base;
    (void)event_id;
    auto wifi_service = static_cast<WiFiService *>(arg);
    auto event = (wifi_event_ap_staconnected_t *)event_data;

    ESP_LOGI(TAG_WiFiService_AccessPoint, "client mac :" MACSTR, MAC2STR(event->mac));
    xTaskNotify(wifi_service->task_handle, CONNECTED_BIT, eNotifyAction::eSetBits);
}