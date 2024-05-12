
#include "settings.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "redundant_storage.h"

static const char *storage_name = "settings";

device_settings *settings = NULL;

void load_settings() {
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       storage_name);
    settings = (device_settings *)storage.data;
}

void store_settings() {
    if (settings == NULL) {
        return;
    }

    redundant_storage storage;
    storage.data = (uint8_t *)settings;
    storage.size = sizeof(*settings);

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            storage_name,
                            storage);
}