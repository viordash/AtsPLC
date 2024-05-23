
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
}

#include "MigrateAnyData.h"
#include "MigrateSettings.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "redundant_storage.h"
#include "settings.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "settings";

device_settings settings = {};

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
        ESP_LOGI(TAG, "Settings. migrated");
    } else if (migrateResult == MigrateRes_Skipped && storedData != NULL) {
        memcpy(&settings, storedData, sizeof(settings));
        ESP_LOGI(TAG, "Settings. loaded");
    } else {
        ESP_LOGE(TAG, "Settings. migrate error");
    }

    delete[] storage.data;
    xSemaphoreGive(mutex);
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

void lock_settings() {
    ESP_ERROR_CHECK(xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE ? ESP_ERR_NO_MEM : ESP_OK);
}

void unlock_settings() {
    xSemaphoreGive(mutex);
}