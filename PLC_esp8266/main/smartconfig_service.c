
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
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

extern device_settings settings;

static void start_smartconfig() {
    vTaskDelay(3000 / portTICK_RATE_MS);
    ESP_LOGW(TAG, "Start process");
}

static void smartconfig_task(void *parm) {
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
        start_smartconfig();
    }
    vTaskDelete(NULL);
}

void try_smartconfig() {
    xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
}