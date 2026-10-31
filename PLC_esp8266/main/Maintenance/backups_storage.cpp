
#include "backups_storage.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_B = "backups_storage";
static const char *partition = backups_storage_name;
static const char *path = backups_path;

bool backups_storage_load(const char *name, backups_storage *storage) {
    ESP_LOGI(TAG_B, "backups_storage load path:'%s', name:'%s'", path, name);

    char filename[256];

    open_storage(partition, path);
    snprintf(filename, sizeof(filename), "%s/%s", path, name);
    bool ready = redundant_storage_read_copy(filename, storage);
    close_storage(partition);
    return ready;
}

void backups_storage_store(const char *name, backups_storage *storage) {
    ESP_LOGI(TAG_B, "backups_storage_store load path:'%s', name:'%s'", path, name);
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path, name);

    open_storage(partition, path);
    redundant_storage_write_copy(filename, storage);
    close_storage(partition);
}

void backups_storage_delete(const char *name) {
    ESP_LOGI(TAG_B, "backups_storage_delete load path:'%s', name:'%s'", path, name);
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path, name);

    open_storage(partition, path);
    if (remove(filename) != 0) {
        ESP_LOGE(TAG_B, "Error deleting file '%s'", filename);
    }
    close_storage(partition);
}