
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

static struct {
    EventGroupHandle_t event;
} service;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int RUNNED_BIT = BIT2;

static void
event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        return;
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        xEventGroupClearBits(service.event, CONNECTED_BIT);
        return;
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(service.event, CONNECTED_BIT);
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
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
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(service.event, ESPTOUCH_DONE_BIT);
        ESP_LOGI(TAG, "SC_EVENT_SEND_ACK_DONE");
        return;
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
                                     timeout_ms / portTICK_RATE_MS);

        ESP_LOGI(TAG, "process, uxBits:0x%08X", uxBits);

        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
            connected = true;
        }

        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            break;
        }
    } while ((uxBits & (CONNECTED_BIT | ESPTOUCH_DONE_BIT)) != 0);
    stop_wifi();
    if (connected) {
        wifi_config_t wifi_config = {};
        char ssid[sizeof(wifi_config.sta.ssid) + 1] = {};
        char pwd[sizeof(wifi_config.sta.password) + 1] = {};

        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
        memcpy(ssid, wifi_config.sta.ssid, sizeof(ssid));
        memcpy(pwd, wifi_config.sta.password, sizeof(pwd));

        SAFETY_SETTINGS( //
            memcpy(settings.wifi.ssid, wifi_config.sta.ssid, sizeof(settings.wifi.ssid));
            memcpy(settings.wifi.password, wifi_config.sta.password, sizeof(settings.wifi.ssid));
            store_settings(); //
        );
        ESP_LOGI(TAG, "store wifi settings, ssid:%s, pwd:%s", wifi_config.sta.ssid, pwd);
    }
    ESP_LOGW(TAG, "Finish process");
}

static void task(void *parm) {
    ESP_LOGI(TAG, "Start task");

    xEventGroupSetBits(service.event, RUNNED_BIT);

    SAFETY_SETTINGS(                    //
        settings.smartconfig.counter++; //
    );

    bool ready_to_smartconfig = settings.smartconfig.counter == start_smartconfig_counter;

    if (!ready_to_smartconfig) {
        TickType_t ticks_start = 0;
        vTaskDelayUntil(&ticks_start, min_period_ms / portTICK_RATE_MS);
        ESP_LOGI(TAG, "Begin check period, %u", settings.smartconfig.counter);
        SAFETY_SETTINGS(      //
            store_settings(); //
        );
        vTaskDelayUntil(&ticks_start, max_period_ms / portTICK_RATE_MS);
        ESP_LOGI(TAG, "End check period, %u", settings.smartconfig.counter);
    }
    SAFETY_SETTINGS(                      //
        settings.smartconfig.counter = 0; //
        store_settings();                 //
    );

    if (ready_to_smartconfig) {
        start_process();
    }

    xEventGroupClearBits(service.event, RUNNED_BIT);
    vTaskDelete(NULL);
}

void start_smartconfig() {
    service.event = xEventGroupCreate();
    ESP_ERROR_CHECK(xTaskCreate(task, "smartconfig_task", 4096, NULL, 3, NULL) != pdPASS ? ESP_FAIL
                                                                                         : ESP_OK);
}

bool smartconfig_is_runned() {
    if (service.event == NULL) {
        return false;
    }
    EventBits_t uxBits = xEventGroupWaitBits(service.event, RUNNED_BIT, false, false, 0);

    if (uxBits & RUNNED_BIT) {
        ESP_LOGD(TAG, "is_runned, uxBits:0x%08X", uxBits);
    }
    return uxBits & RUNNED_BIT;
}
