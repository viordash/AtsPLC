
#include "redundant_storage.h"
#include "crc32.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "storage.h"
#include <stdlib.h>
#include <sys/stat.h>

typedef struct {
    uint32_t crc;
} redundant_storage_header;

static const char *TAG = "redundant_storage";

static redundant_storage read_file(FILE *file) {
    redundant_storage storage = {};
    if (file == NULL) {
        ESP_LOGE(TAG, "check_file, file not exists");
        return storage;
    }

    struct stat st;

    if (fstat(fileno(file), &st) != 0 || st.st_size < sizeof(redundant_storage_header)) {
        ESP_LOGE(TAG, "check_file, file size wrong");
        return storage;
    }

    redundant_storage_header header;
    if (fread(&header, sizeof(redundant_storage_header), 1, file) != 1) {
        ESP_LOGE(TAG, "check_file, read header error");
        return storage;
    }

    storage.size = st.st_size - sizeof(redundant_storage_header);
    storage.data = malloc(storage.size);

    if (fread(storage.data, 1, storage.size, file) != storage.size) {
        ESP_LOGE(TAG, "check_file, read data error");
        free(storage.data);
        storage.size = 0;
        storage.data = NULL;
        return storage;
    }

    if (header.crc != calc_crc32(CRC32_INIT, storage.data, storage.size)) {
        ESP_LOGW(TAG, "check_file, wrong crc\r\n");
        free(storage.data);
        storage.size = 0;
        storage.data = NULL;
        return storage;
    }

    return storage;
}

static void write_file(const char *path, redundant_storage storage) {
    FILE *file = fopen(path, "wb");

    redundant_storage_header header;
    header.crc = calc_crc32(CRC32_INIT, storage.data, storage.size);

    if (fwrite(&header, sizeof(redundant_storage_header), 1, file) != 1
        || fwrite(storage.data, 1, storage.size, file) != storage.size) {
        ESP_LOGE(TAG, "write_file, write error");
    }

    fclose(file);
}

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1) {
    ESP_LOGI(TAG, "redundant_storage load path_0:'%s', path_1:'%s'", path_0, path_1);

    open_storage(partition_0, path_0);
    open_storage(partition_1, path_1);

    FILE *file_0 = fopen(path_0, "rb");
    FILE *file_1 = fopen(path_1, "rb");

    redundant_storage storage_0 = read_file(file_0);
    redundant_storage storage_1 = read_file(file_1);

    fclose(file_0);
    fclose(file_1);

    if (storage_0.data != NULL && storage_1.data == NULL) {
        write_file(path_1, storage_0);
    }
    if (storage_0.data == NULL && storage_1.data != NULL) {
        write_file(path_0, storage_1);
    }

    close_storage(partition_0);
    close_storage(partition_1);

    return storage_0;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             redundant_storage storage) {

    open_storage(partition_0, path_0);
    write_file(path_0, storage);
    close_storage(partition_0);

    open_storage(partition_1, path_1);
    write_file(path_1, storage);
    close_storage(partition_1);
}
