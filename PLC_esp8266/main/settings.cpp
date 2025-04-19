
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
}

#include "MigrateAnyData/MigrateAnyData.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "redundant_storage.h"
#include "settings.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_settings = "settings";

CurrentSettings::device_settings settings = {};

static SemaphoreHandle_t mutex = NULL;

void load_settings() {
    ESP_ERROR_CHECK(mutex == NULL ? ESP_OK : ESP_ERR_NO_MEM);
    mutex = xSemaphoreCreateMutex();

    uint8_t *storedData = NULL;
    size_t storedSize = 0;
    uint32_t version = INITIAL_VERSION;
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       settings_storage_name);
    if (storage.size > 0) {
        version = storage.version;
        storedData = storage.data;
        storedSize = storage.size;
    }

    TMigrateResult migrateResult = MigrateData::Run(version,
                                                    DEVICE_SETTINGS_VERSION,
                                                    &SettingsMigrations,
                                                    storedData,
                                                    storedSize,
                                                    &settings,
                                                    [](void *parent, void *pItem) -> bool {
                                                        memcpy(parent, pItem, sizeof(settings));
                                                        return true;
                                                    });

    if (migrateResult == MigrateRes_Migrate) {
        ESP_LOGI(TAG_settings, "migrated");
    } else if (migrateResult == MigrateRes_Skipped && storedData != NULL) {
        memcpy(&settings, storedData, sizeof(settings));
        ESP_LOGI(TAG_settings, "loaded");
    } else {
        ESP_LOGE(TAG_settings, "migrate error");
    }

    ESP_LOGI(TAG_settings, "smartconfig.counter:%u", settings.smartconfig.counter);
    ESP_LOGI(TAG_settings,
             "wifi.ssid:%.*s",
             (int)sizeof(settings.wifi_station.ssid) - 1,
             settings.wifi_station.ssid);
    ESP_LOGI(TAG_settings,
             "wifi.password:%.*s",
             (int)sizeof(settings.wifi_station.password) - 1,
             settings.wifi_station.password);

    delete[] storage.data;
}

void store_settings() {
    redundant_storage storage;
    storage.data = (uint8_t *)&settings;
    storage.size = sizeof(settings);
    storage.version = DEVICE_SETTINGS_VERSION;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
}

void delete_settings() {
    redundant_storage_delete(storage_0_partition,
                             storage_0_path,
                             storage_1_partition,
                             storage_1_path,
                             settings_storage_name);

    ESP_LOGI(TAG_settings, "deleted");
}

static bool invalid_symbol(char ch) {
    return ch < '!' || ch > '~';
}

static bool invalid_int(int32_t value, int32_t min, int32_t max) {
    return value < min || value > max;
}

static bool invalid_uint(uint32_t value, uint32_t min, uint32_t max) {
    return value < min || value > max;
}

bool validate_settings(CurrentSettings::device_settings *settings) {
    size_t pos;
    char ch;

    pos = 0;
    while (pos < sizeof(settings->wifi_station.ssid)
           && (ch = settings->wifi_station.ssid[pos]) != 0) {
        if (invalid_symbol(ch)) {
            ESP_LOGI(TAG_settings, "Invalid wifi_station.ssid, '%s'", settings->wifi_station.ssid);
            return false;
        }
        pos++;
    }

    pos = 0;
    while (pos < sizeof(settings->wifi_station.password)
           && (ch = settings->wifi_station.password[pos]) != 0) {
        if (invalid_symbol(ch)) {
            ESP_LOGI(TAG_settings,
                     "Invalid wifi_station.password, '%s'",
                     settings->wifi_station.password);
            return false;
        }
        pos++;
    }
    if (invalid_int(settings->wifi_station.connect_max_retry_count, -1, 7777)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.connect_max_retry_count, '%d'",
                 settings->wifi_station.connect_max_retry_count);
        return false;
    }
    if (invalid_uint(settings->wifi_station.reconnect_delay_ms, 100, 10 * 60 * 1000)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.reconnect_delay_ms, '%u'",
                 settings->wifi_station.reconnect_delay_ms);
        return false;
    }
    if (invalid_uint(settings->wifi_station.scan_station_rssi_period_ms, 100, 10 * 60 * 1000)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.scan_station_rssi_period_ms, '%u'",
                 settings->wifi_station.scan_station_rssi_period_ms);
        return false;
    }
    if (invalid_int(settings->wifi_station.max_rssi, -120, 100)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.max_rssi, '%d'",
                 settings->wifi_station.max_rssi);
        return false;
    }
    if (invalid_int(settings->wifi_station.min_rssi, -120, 100)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.min_rssi, '%d'",
                 settings->wifi_station.min_rssi);
        return false;
    }
    if (settings->wifi_station.min_rssi > settings->wifi_station.max_rssi) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_station.min_rssi > settings->wifi_station.max_rssi, '%d'>'%d'",
                 settings->wifi_station.min_rssi,
                 settings->wifi_station.max_rssi);
        return false;
    }

    if (invalid_uint(settings->wifi_scanner.per_channel_scan_time_ms, 100, 20 * 1000)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_scanner.per_channel_scan_time_ms, '%u'",
                 settings->wifi_scanner.per_channel_scan_time_ms);
        return false;
    }
    if (invalid_int(settings->wifi_scanner.max_rssi, -120, 100)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_scanner.max_rssi, '%d'",
                 settings->wifi_scanner.max_rssi);
        return false;
    }
    if (invalid_int(settings->wifi_scanner.min_rssi, -120, 100)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_scanner.min_rssi, '%d'",
                 settings->wifi_scanner.min_rssi);
        return false;
    }
    if (settings->wifi_scanner.min_rssi > settings->wifi_scanner.max_rssi) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_scanner.min_rssi > settings->wifi_scanner.max_rssi, '%d'>'%d'",
                 settings->wifi_scanner.min_rssi,
                 settings->wifi_scanner.max_rssi);
        return false;
    }

    if (invalid_uint(settings->wifi_access_point.generation_time_ms, 100, 10 * 60 * 1000)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_access_point.generation_time_ms, '%u'",
                 settings->wifi_access_point.generation_time_ms);
        return false;
    }
    if (invalid_uint(settings->wifi_access_point.ssid_hidden, 0, 1)) {
        ESP_LOGI(TAG_settings,
                 "Invalid wifi_access_point.ssid_hidden, '%u'",
                 settings->wifi_access_point.ssid_hidden);
        return false;
    }
    return true;
}

void lock_settings() {
    ESP_ERROR_CHECK(xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE ? ESP_ERR_NO_MEM : ESP_OK);
}

void unlock_settings() {
    xSemaphoreGive(mutex);
}