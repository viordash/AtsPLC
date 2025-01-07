
#include "storage.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG_storage = "storage";

void open_storage(const char *partition, const char *path) {
    ESP_LOGD(TAG_storage, "Initializing storage '%s'", partition);

    esp_vfs_spiffs_conf_t conf = { .base_path = path,
                                   .partition_label = partition,
                                   .max_files = 3,
                                   .format_if_mount_failed = true };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret == ESP_FAIL) {
        ESP_LOGE(TAG_storage, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
        ESP_LOGE(TAG_storage, "Failed to find SPIFFS partition");
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG_storage, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
}

void close_storage(const char *partition) {
    esp_vfs_spiffs_unregister(partition);
    ESP_LOGD(TAG_storage, "unmount storage '%s'", partition);
}