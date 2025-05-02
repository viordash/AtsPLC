
#include "LogicProgram/Bindings/WiFiApBinding.h"
#include "LogicProgram/Controller.h"
#include "WiFiService.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "settings.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>

static const char *TAG_WiFiService_AccessPoint = "WiFiService.AccessPoint";
extern CurrentSettings::device_settings settings;

void WiFiService::AccessPointTask(RequestItem *request) {
    AccessPointEventArg ap_event_arg = { this,
                                         request->Payload.AccessPoint.ssid,
                                         request->Payload.AccessPoint.mac };
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
        ESP_LOGE(TAG_WiFiService_AccessPoint, "esp_wifi_set_mode err 0x%X", (unsigned int)err);
        return;
    }

    err = esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WiFiService_AccessPoint, "esp_wifi_set_config err 0x%X", (unsigned int)err);
        return;
    }

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_AP_STACONNECTED,
                                               &ap_connect_wifi_event_handler,
                                               &ap_event_arg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_AP_STADISCONNECTED,
                                               &ap_disconnect_wifi_event_handler,
                                               &ap_event_arg));

    ESP_ERROR_CHECK(esp_wifi_start());

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

        ESP_LOGD(TAG_WiFiService_AccessPoint,
                 "process, uxBits:0x%08X",
                 (unsigned int)ulNotifiedValue);

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
    }

    esp_wifi_stop();

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                                                 WIFI_EVENT_AP_STACONNECTED,
                                                 &ap_connect_wifi_event_handler));

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                                                 WIFI_EVENT_AP_STADISCONNECTED,
                                                 &ap_disconnect_wifi_event_handler));

    requests.RemoveAccessPoint(request->Payload.AccessPoint.ssid);
    if (!cancel) {
        requests.AccessPoint(request->Payload.AccessPoint.ssid,
                             request->Payload.AccessPoint.password,
                             request->Payload.AccessPoint.mac);
    }

    ESP_LOGI(TAG_WiFiService_AccessPoint, "finish");
}

void WiFiService::ap_connect_wifi_event_handler(void *arg,
                                                esp_event_base_t event_base,
                                                int32_t event_id,
                                                void *event_data) {
    (void)event_base;
    (void)event_id;
    auto ap_event_arg = static_cast<AccessPointEventArg *>(arg);
    auto event = (wifi_event_ap_staconnected_t *)event_data;

    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "connect client aid:%u, mac:" MACSTR ", mask:%s",
             (unsigned int)event->aid,
             MAC2STR(event->mac),
             ap_event_arg->mac);

    if (WiFiApBinding::ClientMacMatches(ap_event_arg->mac, event->mac)) {
        t_mac mac = {};
        memcpy(&mac, &event->mac, sizeof(event->mac));
        ap_event_arg->service->AddApClient(ap_event_arg->ssid, mac);
        Controller::WakeupProcessTask();
    } else {
        esp_wifi_deauth_sta(event->aid);
    }
}

void WiFiService::ap_disconnect_wifi_event_handler(void *arg,
                                                   esp_event_base_t event_base,
                                                   int32_t event_id,
                                                   void *event_data) {
    (void)event_base;
    (void)event_id;
    auto ap_event_arg = static_cast<AccessPointEventArg *>(arg);
    auto event = (wifi_event_ap_stadisconnected_t *)event_data;

    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "disconnect client aid:%u, mac:" MACSTR ", mask:%s",
             (unsigned int)event->aid,
             MAC2STR(event->mac),
             ap_event_arg->mac);

    if (WiFiApBinding::ClientMacMatches(ap_event_arg->mac, event->mac)) {
        t_mac mac = {};
        memcpy(&mac, &event->mac, sizeof(event->mac));
        ap_event_arg->service->RemoveApClient(ap_event_arg->ssid, mac);
        Controller::WakeupProcessTask();
    }
}

void WiFiService::AddApClient(const char *ssid, t_mac mac) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.insert({ ssid, { mac } });
    if (!it.second) {
        it.first->second.insert(mac);
    }
    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "AddApClient, ssid_cnt: %u, ssid_clients:%u",
             (unsigned int)ap_clients.size(),
             (unsigned int)it.first->second.size());
}

size_t WiFiService::GetApClientsCount(const char *ssid) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.find(ssid);
    bool found = it != ap_clients.end();
    ESP_LOGD(TAG_WiFiService_AccessPoint, "FindApClient, found:%u", (unsigned int)found);
    if (found) {
        return it->second.size();
    }
    return 0;
}

void WiFiService::RemoveApClient(const char *ssid, t_mac mac) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    auto it = ap_clients.find(ssid);
    bool found = it != ap_clients.end();
    ESP_LOGD(TAG_WiFiService_AccessPoint, "FindApClient, found:%u", (unsigned int)found);
    if (found) {
        it->second.erase(mac);
        bool empty_ssid_to_be_delete = it->second.size() == 0;
        if (empty_ssid_to_be_delete) {
            ap_clients.erase(ssid);
        }
    }
    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "RemoveApClient, cnt:%u",
             (unsigned int)ap_clients.size());
}

void WiFiService::RemoveApClients(const char *ssid) {
    std::lock_guard<std::mutex> lock(ap_clients_lock_mutex);
    ap_clients.erase(ssid);
    ESP_LOGI(TAG_WiFiService_AccessPoint,
             "RemoveApClients, cnt:%u",
             (unsigned int)ap_clients.size());
}
