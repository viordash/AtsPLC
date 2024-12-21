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

static const char *TAG_WiFiService = "WiFiService";
extern device_settings settings;

WiFiService::WiFiService() {
}

WiFiService::~WiFiService() {
    Stop();
}

void WiFiService::Start() {
    ESP_LOGW(TAG_WiFiService, "Start");
    event = xEventGroupCreate();
    xEventGroupSetBits(event, WiFiService::STARTED_BIT);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, this));

    ESP_ERROR_CHECK(xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, 3, NULL) != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void WiFiService::Stop() {
    ESP_LOGW(TAG_WiFiService, "Stop");
    if (!Started()) {
        return;
    }

    xEventGroupSetBits(event, WiFiService::STOP_BIT);

    EventBits_t uxBits = xEventGroupWaitBits(event, WiFiService::RUNNED_BIT, false, false, 0);
    bool runned = (uxBits & WiFiService::RUNNED_BIT) != 0;
    if (runned) {
        ESP_LOGI(TAG_WiFiService, "Stop, is_runned, uxBits:0x%08X", uxBits);

        uxBits = xEventGroupWaitBits(event, WiFiService::STOPPED_BIT, false, false, portMAX_DELAY);
        if (uxBits & WiFiService::STOPPED_BIT) {
            ESP_LOGI(TAG_WiFiService, "stopped, uxBits:0x%08X", uxBits);
        }
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler));
    ESP_ERROR_CHECK(
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));

    Disconnect();
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(tcpip_adapter_clear_default_wifi_handlers());

    xEventGroupClearBits(event, WiFiService::STARTED_BIT);
    vEventGroupDelete(event);
}

bool WiFiService::Started() {
    EventBits_t uxBits = xEventGroupWaitBits(event, WiFiService::STARTED_BIT, false, false, 0);

    if (uxBits & WiFiService::STARTED_BIT) {
        ESP_LOGI(TAG_WiFiService, "is_runned, uxBits:0x%08X", uxBits);
    }
    return uxBits & WiFiService::STARTED_BIT;
}

bool WiFiService::Scan(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto res = requests.insert({ RequestItemType::wqi_ScanSsid, ssid, false });
    bool was_inserted = res.second;
    if (was_inserted) {
        xEventGroupSetBits(event, WiFiService::NEW_REQUEST_BIT);
        return false;
    }
    return res.first->Payload.ScanSsid.status;
}

void WiFiService::Generate(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto res = requests.insert({ RequestItemType::wqi_ScanSsid, ssid, false });
    bool was_inserted = res.second;
    if (was_inserted) {
        xEventGroupSetBits(event, WiFiService::NEW_REQUEST_BIT);
    }
}

void WiFiService::Connect() {
    ESP_LOGI(TAG_WiFiService, "connect");

    wifi_config_t wifi_config = {};
    SAFETY_SETTINGS( //
        memcpy(wifi_config.sta.ssid, settings.wifi.ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password,
               settings.wifi.password,
               sizeof(wifi_config.sta.password)); //
    );

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
     * However these modes are deprecated and not advisable to be used. Incase your Access point
     * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (wifi_config.sta.password[0] != 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiService::Disconnect() {
    ESP_LOGI(TAG_WiFiService, "disconnect");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

void WiFiService::Task(void *parm) {
    ESP_LOGI(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    xEventGroupSetBits(wifi_service->event, WiFiService::RUNNED_BIT);

    int connect_retry_num = 0;
    EventBits_t uxBits;
    do {
        TickType_t start_loop_ticks = xTaskGetTickCount();

        wifi_service->Connect();

        uxBits = xEventGroupWaitBits(wifi_service->event,
                                     WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                         | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        ESP_LOGI(TAG_WiFiService, "process, uxBits:0x%08X", uxBits);

        if ((uxBits & CONNECTED_BIT) != 0) {
            ESP_LOGI(TAG_WiFiService, "Connected to ap");
            connect_retry_num = 0;
        } else if ((uxBits & WiFiService::STOP_BIT) == 0) {
            wifi_service->Disconnect();
        }

        int32_t max_retry_count;
        SAFETY_SETTINGS(                                             //
            max_retry_count = settings.wifi.connect_max_retry_count; //
        );

        if ((uxBits & FAILED_BIT) != 0) {
            bool retry_connect =
                (max_retry_count == INFINITY_CONNECT_RETRY || connect_retry_num < max_retry_count);

            if (retry_connect) {
                connect_retry_num++;
                ESP_LOGI(TAG_WiFiService,
                         "failed. reconnect, num:%d of %d",
                         connect_retry_num,
                         max_retry_count);

                vTaskDelayUntil(&start_loop_ticks, 1000 / portTICK_RATE_MS);
            } else {
                ESP_LOGI(TAG_WiFiService, "failed. unable reconnect");
                break;
            }
        }

    } while (uxBits != 0 && (uxBits & WiFiService::STOP_BIT) == 0);

    ESP_LOGW(TAG_WiFiService, "Finish task");
    xEventGroupSetBits(wifi_service->event, WiFiService::STOPPED_BIT);
    vTaskDelete(NULL);
}

void WiFiService::wifi_event_handler(void *arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id,
                                     void *event_data) {
    (void)event_data;
    auto wifi_service = static_cast<WiFiService *>(arg);
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG_WiFiService, "start wifi event");
            esp_wifi_connect();
            return;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG_WiFiService, "connect to the AP fail");
            xEventGroupSetBits(wifi_service->event, WiFiService::FAILED_BIT);
            return;

        case WIFI_EVENT_STA_STOP:
            ESP_LOGI(TAG_WiFiService, "stop wifi event");
            return;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG_WiFiService, "wifi connected event");
            return;

        default:
            ESP_LOGW(TAG_WiFiService,
                     "unhandled event, event_base:%s, event_id:%d",
                     event_base,
                     event_id);
            break;
    }
}

void WiFiService::ip_event_handler(void *arg,
                                   esp_event_base_t event_base,
                                   int32_t event_id,
                                   void *event_data) {
    auto wifi_service = static_cast<WiFiService *>(arg);
    auto event = (ip_event_got_ip_t *)event_data;

    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG_WiFiService, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
            xEventGroupSetBits(wifi_service->event, CONNECTED_BIT);
            return;

        default:
            ESP_LOGW(TAG_WiFiService,
                     "unhandled event, event_base:%s, event_id:%d",
                     event_base,
                     event_id);
            break;
    }
}
