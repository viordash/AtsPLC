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

    ConnectToStation();
    ESP_ERROR_CHECK(xTaskCreate(WiFiService::Task, "wifi_task", 2048, this, 3, NULL) != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);
}

void WiFiService::Stop() {
    ESP_LOGW(TAG_WiFiService, "Stop");
    if (!Started()) {
        return;
    }

    xEventGroupSetBits(event, STOP_BIT);

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

RequestItem WiFiService::PopRequest() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return requests.Pop();
}

void WiFiService::ConnectToStation() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    RequestItem request = {};
    request.type = RequestItemType::wqi_Station;
    auto it = requests.Add(&request);
    bool was_inserted = it == requests.end();
    if (was_inserted) {
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
    }
}

bool WiFiService::Scan(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scanner;
    request.Payload.Scanner.ssid = ssid;
    request.Payload.Scanner.status = false;

    auto it = requests.Add(&request);
    bool was_inserted = it == requests.end();
    if (was_inserted) {
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
        return false;
    }
    return it->Payload.Scanner.status;
}

void WiFiService::Generate(const char *ssid) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    RequestItem request = {};
    request.type = RequestItemType::wqi_AccessPoint;
    request.Payload.AccessPoint.ssid = ssid;

    auto it = requests.Add(&request);
    bool was_inserted = it == requests.end();
    if (was_inserted) {
        xEventGroupSetBits(event, NEW_REQUEST_BIT);
    }
}

void WiFiService::Task(void *parm) {
    ESP_LOGI(TAG_WiFiService, "Start task");
    auto wifi_service = static_cast<WiFiService *>(parm);

    EventBits_t uxBits;
    do {
        uxBits = xEventGroupWaitBits(wifi_service->event,
                                     STOP_BIT | NEW_REQUEST_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        if ((uxBits & NEW_REQUEST_BIT) != 0) {

            RequestItem new_request = wifi_service->PopRequest();
            ESP_LOGI(TAG_WiFiService, "New request, type:%u", new_request.type);

            switch (new_request.type) {
                case wqi_Station:
                    uxBits = wifi_service->StationTask();
                    break;

                default:
                    break;
            }
        }

    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

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