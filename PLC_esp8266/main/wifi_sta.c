#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "settings.h"
#include "tcpip_adapter.h"
#include "wifi_sta.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "wifi sta";
extern device_settings settings;

#define INFINITY_CONNECT_RETRY -1

static struct {
    EventGroupHandle_t event;
    int connect_retry_num;
} service;

static const int FAILED_BIT = BIT0;
static const int CONNECTED_BIT = BIT1;
static const int RUNNED_BIT = BIT2;
static const int STOP_BIT = BIT3;

static void
event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "start wifi event");
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "connect to the AP fail");
        xEventGroupSetBits(service.event, FAILED_BIT);
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
        ESP_LOGI(TAG, "stop wifi event");
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "wifi connected event");
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
        xEventGroupSetBits(service.event, CONNECTED_BIT);
        return;
    }
    ESP_LOGW(TAG, "unhandled event, event_base:%s, event_id:%d", event_base, event_id);
}

static void connect() {
    ESP_LOGI(TAG, "connect");

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

static void disconnect() {
    ESP_LOGI(TAG, "disconnect");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
}

static void task(void *parm) {
    ESP_LOGI(TAG, "Start task");
    xEventGroupSetBits(service.event, RUNNED_BIT);

    service.connect_retry_num = 0;
    EventBits_t uxBits;
    do {

        TickType_t connect_ticks = xTaskGetTickCount();

        connect();

        uxBits = xEventGroupWaitBits(service.event,
                                     CONNECTED_BIT | FAILED_BIT | STOP_BIT,
                                     true,
                                     false,
                                     portMAX_DELAY);

        ESP_LOGI(TAG, "process, uxBits:0x%08X", uxBits);

        if ((uxBits & CONNECTED_BIT) != 0) {
            ESP_LOGI(TAG, "Connected to ap");
            service.connect_retry_num = 0;
        } else if ((uxBits & STOP_BIT) == 0) {
            disconnect();
        }

        int32_t max_retry_count;
        SAFETY_SETTINGS(                                             //
            max_retry_count = settings.wifi.connect_max_retry_count; //
        );

        if ((uxBits & FAILED_BIT) != 0) {
            bool retry_connect = (max_retry_count == INFINITY_CONNECT_RETRY
                                  || service.connect_retry_num < max_retry_count);

            if (retry_connect) {
                service.connect_retry_num++;
                ESP_LOGI(TAG,
                         "failed. reconnect, num:%d of %d",
                         service.connect_retry_num,
                         max_retry_count);

                vTaskDelayUntil(&connect_ticks, 1000 / portTICK_RATE_MS);
            } else {
                ESP_LOGI(TAG, "failed. unable reconnect");
                break;
            }
        }

    } while (uxBits != 0 && (uxBits & STOP_BIT) == 0);

    xEventGroupClearBits(service.event, RUNNED_BIT);
    ESP_LOGW(TAG, "Finish task");
    vTaskDelete(NULL);
}

void start_wifi_sta() {
    ESP_LOGW(TAG, "start_wifi_sta");
    service.event = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(xTaskCreate(task, "wifi_sta_task", 4096, NULL, 3, NULL) != pdPASS ? ESP_FAIL
                                                                                      : ESP_OK);
}

void stop_wifi_sta() {
    ESP_LOGW(TAG, "stop_wifi_sta");
    if (!wifi_sta_is_runned()) {
        return;
    }

    xEventGroupSetBits(service.event, STOP_BIT);
    xEventGroupClearBits(service.event, RUNNED_BIT);

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));

    disconnect();
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(tcpip_adapter_clear_default_wifi_handlers());

    EventGroupHandle_t event = service.event;
    service.event = NULL;
    vEventGroupDelete(event);
}

bool wifi_sta_is_runned() {
    if (service.event == NULL) {
        return false;
    }
    EventBits_t uxBits = xEventGroupWaitBits(service.event, RUNNED_BIT, false, false, 0);

    if (uxBits & RUNNED_BIT) {
        ESP_LOGI(TAG, "is_runned, uxBits:0x%08X", uxBits);
    }
    return uxBits & RUNNED_BIT;
}