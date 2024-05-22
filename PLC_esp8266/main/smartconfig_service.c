
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "settings.h"
#include "smartconfig_ack.h"
#include "smartconfig_service.h"
#include "tcpip_adapter.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "smartconfig";
static const uint32_t min_period_ms = 500;
static const uint32_t max_period_ms = 4000;
static const uint32_t start_smartconfig_counter = 4;
static const uint32_t timeout_ms = 600000;

extern device_settings settings;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

static void
event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config = {};

        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;

        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            uint8_t rvd_data[65] = {};
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:%s", rvd_data);
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
        ESP_LOGI(TAG, "SC_EVENT_SEND_ACK_DONE");
    }
}

static void initialize_wifi(void) {
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void stop_wifi(void) {
    esp_smartconfig_stop();
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler));
}

static void start_smartconfig(EventGroupHandle_t smartconfig_ready_event) {
    ESP_LOGW(TAG, "Start process");
    s_wifi_event_group = xEventGroupCreate();

    initialize_wifi();

    EventBits_t uxBits = ~(CONNECTED_BIT | ESPTOUCH_DONE_BIT);
    bool connected = false;
    while (uxBits != 0) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group,
                                     CONNECTED_BIT | ESPTOUCH_DONE_BIT,
                                     true,
                                     false,
                                     timeout_ms / portTICK_RATE_MS);

        ESP_LOGI(TAG, "process, uxBits:0x%08X", uxBits);

        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
            connected = true;
        }

        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            break;
        }
    }

    stop_wifi();
    vEventGroupDelete(s_wifi_event_group);

    if (connected) {
        wifi_config_t wifi_config = {};
        char ssid[sizeof(wifi_config.sta.ssid) + 1] = {};
        char pwd[sizeof(wifi_config.sta.password) + 1] = {};

        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
        memcpy(ssid, wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid));
        memcpy(pwd, wifi_config.sta.password, sizeof(wifi_config.sta.password));
        ESP_LOGI(TAG, "store wifi settings, ssid:%s, pwd:%s", wifi_config.sta.ssid, pwd);

        xEventGroupSetBits(smartconfig_ready_event, smartconfig_ready_bit);
    }
    ESP_LOGW(TAG, "Finish process");
}

static void smartconfig_task(void *parm) {
    EventGroupHandle_t smartconfig_ready_event = (EventGroupHandle_t)parm;
    ESP_LOGI(TAG, "Start task");

    settings.smartconfig.counter++;
    bool ready_to_smartconfig = settings.smartconfig.counter == start_smartconfig_counter;

    if (!ready_to_smartconfig) {
        TickType_t ticks_start = 0;
        vTaskDelayUntil(&ticks_start, min_period_ms / portTICK_RATE_MS);
        ESP_LOGI(TAG, "Begin check period, %u", settings.smartconfig.counter);
        store_settings();
        vTaskDelayUntil(&ticks_start, max_period_ms / portTICK_RATE_MS);
        ESP_LOGI(TAG, "End check period, %u", settings.smartconfig.counter);
    }
    settings.smartconfig.counter = 0;
    store_settings();

    if (ready_to_smartconfig) {
        start_smartconfig(smartconfig_ready_event);
    }
    vTaskDelete(NULL);
}

EventGroupHandle_t try_smartconfig() {
    EventGroupHandle_t smartconfig_ready_event = xEventGroupCreate();
    ESP_ERROR_CHECK(
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, smartconfig_ready_event, 3, NULL)
                != pdPASS
            ? ESP_FAIL
            : ESP_OK);
    return smartconfig_ready_event;
}