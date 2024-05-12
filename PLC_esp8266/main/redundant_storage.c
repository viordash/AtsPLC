
#include "redundant_storage.h"
#include "crc32.h"
#include "esp_err.h"
#include "esp_log.h"
#include "storage.h"
#include <stdlib.h>
#include <sys/stat.h>



typedef struct {
    uint32_t crc;
} redundant_storage_header;

static const char *TAG_R = "redundant_storage";

static redundant_storage read_file(FILE *file) {
    redundant_storage storage = {};
    if (file == NULL) {
        ESP_LOGE(TAG_R, "check_file, file not exists");
        return storage;
    }

    struct stat st;

    if (fstat(fileno(file), &st) != 0 || (size_t)st.st_size < sizeof(redundant_storage_header)) {
        ESP_LOGE(TAG_R, "check_file, file size wrong");
        return storage;
    }

    redundant_storage_header header;
    if (fread(&header, sizeof(redundant_storage_header), 1, file) != 1) {
        ESP_LOGE(TAG_R, "check_file, read header error");
        return storage;
    }

    storage.size = st.st_size - sizeof(redundant_storage_header);
    storage.data = malloc(storage.size);

    if (fread(storage.data, 1, storage.size, file) != storage.size) {
        ESP_LOGE(TAG_R, "check_file, read data error");
        free(storage.data);
        storage.size = 0;
        storage.data = NULL;
        return storage;
    }

    if (header.crc != calc_crc32(CRC32_INIT, storage.data, storage.size)) {
        ESP_LOGW(TAG_R, "check_file, wrong crc\r\n");
        free(storage.data);
        storage.size = 0;
        storage.data = NULL;
        return storage;
    }

    return storage;
}

static void write_file(const char *path, redundant_storage storage) {
    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        ESP_LOGE(TAG_R, "write_file, file open error");
        return;
    }

    redundant_storage_header header;
    header.crc = calc_crc32(CRC32_INIT, storage.data, storage.size);

    if (fwrite(&header, sizeof(redundant_storage_header), 1, file) != 1
        || fwrite(storage.data, 1, storage.size, file) != storage.size) {
        ESP_LOGE(TAG_R, "write_file, write error");
    }

    fclose(file);
}

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1,
                                         const char *name) {
    ESP_LOGI(TAG_R,
             "redundant_storage load path_0:'%s', path_1:'%s', name:'%s'",
             path_0,
             path_1,
             name);

    char filename_0[256];
    char filename_1[256];

    open_storage(partition_0, path_0);
    snprintf(filename_0, sizeof(filename_0), "%s/%s", path_0, name);
    FILE *file_0 = fopen(filename_0, "rb");
    redundant_storage storage_0 = read_file(file_0);

    if (file_0 != NULL) {
        fclose(file_0);
    }
    close_storage(partition_0);

    open_storage(partition_1, path_1);
    snprintf(filename_1, sizeof(filename_1), "%s/%s", path_1, name);
    FILE *file_1 = fopen(filename_1, "rb");
    redundant_storage storage_1 = read_file(file_1);

    if (file_1 != NULL) {
        fclose(file_1);
    }
    close_storage(partition_1);

    if (storage_0.data != NULL) {
        if (storage_1.data == NULL) {
            write_file(filename_1, storage_0);
        } else {
            free(storage_1.data);
        }
    }

    if (storage_0.data == NULL && storage_1.data != NULL) {
        write_file(filename_0, storage_1);
        storage_0 = storage_1;
    }

    return storage_0;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage storage) {

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path_0, name);
    open_storage(partition_0, path_0);
    write_file(filename, storage);
    close_storage(partition_0);

    snprintf(filename, sizeof(filename), "%s/%s", path_1, name);
    open_storage(partition_1, path_1);
    write_file(filename, storage);
    close_storage(partition_1);
}
