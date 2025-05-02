
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
#include "sys_gpio.h"
#include "tcpip_adapter.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "smartconfig";

static const uint32_t timeout_ms = 600000;

extern CurrentSettings::device_settings settings;

static struct {
    EventGroupHandle_t event;
} service;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int STATUS_START_BIT = BIT2;
static const int STATUS_STARTED_BIT = BIT3;
static const int STATUS_DISCONNECTED_BIT = BIT4;
static const int STATUS_GOT_IP_BIT = BIT5;
static const int STATUS_SCAN_DONE_BIT = BIT6;
static const int STATUS_FOUND_CHANNEL_BIT = BIT7;
static const int STATUS_GOT_CREDS_BIT = BIT8;
static const int STATUS_COMPLETED_BIT = BIT9;
static const int STATUS_ERROR_BIT = BIT10;

#define STATUS_ALL_BITS                                                                            \
    (STATUS_START_BIT | STATUS_STARTED_BIT | STATUS_DISCONNECTED_BIT | STATUS_GOT_IP_BIT           \
     | STATUS_SCAN_DONE_BIT | STATUS_FOUND_CHANNEL_BIT | STATUS_GOT_CREDS_BIT                      \
     | STATUS_COMPLETED_BIT | STATUS_ERROR_BIT)

static void
event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(service.event, STATUS_STARTED_BIT);
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        return;
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(service.event, CONNECTED_BIT);
        xEventGroupSetBits(service.event, STATUS_DISCONNECTED_BIT);
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED. restart smartconfig");
        ESP_ERROR_CHECK(esp_smartconfig_stop());
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
        return;
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(service.event, CONNECTED_BIT | STATUS_GOT_IP_BIT);
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        xEventGroupSetBits(service.event, STATUS_SCAN_DONE_BIT);
        ESP_LOGI(TAG, "Scan done");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        xEventGroupSetBits(service.event, STATUS_FOUND_CHANNEL_BIT);
        ESP_LOGI(TAG, "Found channel");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");
        xEventGroupSetBits(service.event, STATUS_GOT_CREDS_BIT);

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
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(service.event, ESPTOUCH_DONE_BIT | STATUS_COMPLETED_BIT);
        ESP_LOGI(TAG, "SC_EVENT_SEND_ACK_DONE");
        return;
    }
}

static void initialize_wifi(void) {
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
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler));

    esp_smartconfig_stop();
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(tcpip_adapter_clear_default_wifi_handlers());
}

static void start_process() {
    ESP_LOGW(TAG, "Start process");

    initialize_wifi();

    EventBits_t uxBits;
    bool connected = false;
    do {
        uxBits = xEventGroupWaitBits(service.event,
                                     CONNECTED_BIT | ESPTOUCH_DONE_BIT,
                                     true,
                                     false,
                                     timeout_ms / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "process, uxBits:0x%08X", (unsigned int)uxBits);

        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
            connected = true;
        }

        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            break;
        }
    } while ((uxBits & (CONNECTED_BIT | ESPTOUCH_DONE_BIT)) != 0);

    if (connected) {
        wifi_config_t wifi_config = {};
        char ssid[sizeof(wifi_config.sta.ssid) + 1] = {};
        char pwd[sizeof(wifi_config.sta.password) + 1] = {};

        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
        memcpy(ssid, wifi_config.sta.ssid, sizeof(ssid));
        memcpy(pwd, wifi_config.sta.password, sizeof(pwd));

        SAFETY_SETTINGS( //
            memcpy(settings.wifi_station.ssid,
                   wifi_config.sta.ssid,
                   sizeof(settings.wifi_station.ssid));
            memcpy(settings.wifi_station.password,
                   wifi_config.sta.password,
                   sizeof(settings.wifi_station.ssid));
            store_settings(); //
        );
        ESP_LOGI(TAG, "store wifi settings, ssid:%s, pwd:%s", wifi_config.sta.ssid, pwd);
    } else {
        xEventGroupSetBits(service.event, STATUS_ERROR_BIT);
    }

    stop_wifi();
    ESP_LOGW(TAG, "Finish process");
}

static void task(void *parm) {
    ESP_LOGI(TAG, "Start task");

    xEventGroupSetBits(service.event, STATUS_START_BIT);

    start_process();

    ESP_LOGW(TAG, "Finish task");
    vTaskDelete(NULL);
}

void start_smartconfig() {
    ESP_LOGI(TAG, "start_smartconfig");
    service.event = xEventGroupCreate();
    ESP_ERROR_CHECK(xTaskCreate(task, "smartconfig_task", 2048, NULL, 3, NULL) != pdPASS ? ESP_FAIL
                                                                                         : ESP_OK);
}

void finish_smartconfig() {
    vEventGroupDelete(service.event);
    ESP_LOGI(TAG, "finish_smartconfig");
}

enum SmartconfigStatus smartconfig_status() {
    if (service.event == NULL) {
        return scs_Error;
    }

    EventBits_t uxBits =
        xEventGroupWaitBits(service.event, STATUS_ALL_BITS, true, false, portMAX_DELAY);

    if (uxBits & STATUS_START_BIT) {
        return scs_Start;
    }
    if (uxBits & STATUS_STARTED_BIT) {
        return scs_Started;
    }
    if (uxBits & STATUS_DISCONNECTED_BIT) {
        return scs_Disconnected;
    }
    if (uxBits & STATUS_GOT_IP_BIT) {
        return scs_GotIP;
    }
    if (uxBits & STATUS_SCAN_DONE_BIT) {
        return scs_ScanDone;
    }
    if (uxBits & STATUS_FOUND_CHANNEL_BIT) {
        return scs_FoundChannel;
    }
    if (uxBits & STATUS_GOT_CREDS_BIT) {
        return scs_GotCreds;
    }
    if (uxBits & STATUS_COMPLETED_BIT) {
        return scs_Completed;
    }
    return scs_Error;
}