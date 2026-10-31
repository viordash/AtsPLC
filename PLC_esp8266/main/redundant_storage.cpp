
#include "redundant_storage.h"
#include "crc32.h"
#include "esp_err.h"
#include "esp_log.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    uint32_t crc;
} redundant_storage_header;

static const char *TAG_R = "redundant_storage";

static bool read_file(FILE *file, redundant_storage *storage) {
    if (file == NULL) {
        ESP_LOGE(TAG_R, "check_file, file not exists");
        return false;
    }

    struct stat st;

    if (fstat(fileno(file), &st) != 0 || (size_t)st.st_size < sizeof(redundant_storage_header)) {
        ESP_LOGE(TAG_R, "check_file, file size wrong");
        return false;
    }

    redundant_storage_header header;
    if (fread(&header, 1, sizeof(redundant_storage_header), file)
        != sizeof(redundant_storage_header)) {
        ESP_LOGE(TAG_R, "check_file, read header error");
        return false;
    }

    if (fread(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)) {
        ESP_LOGE(TAG_R, "check_file, read version error");
        return false;
    }
    uint32_t crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));

    storage->size = st.st_size - sizeof(redundant_storage_header) - sizeof(storage->version);
    storage->data = new uint8_t[storage->size];

    if (fread(storage->data, 1, storage->size, file) != storage->size) {
        ESP_LOGE(TAG_R, "check_file, read data error");
        delete[] storage->data;
        return false;
    }

    if (header.crc != calc_crc32(crc, storage->data, storage->size)) {
        ESP_LOGW(TAG_R, "check_file, wrong crc\r");
        delete[] storage->data;
        return false;
    }

    return true;
}

void redundant_storage_write_copy(const char *filename, redundant_storage *storage) {
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        ESP_LOGE(TAG_R, "write_file, file open error");
        return;
    }

    redundant_storage_header header;

    header.crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));
    header.crc = calc_crc32(header.crc, storage->data, storage->size);

    if (fwrite(&header, 1, sizeof(redundant_storage_header), file)
            != sizeof(redundant_storage_header)
        || fwrite(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)
        || fwrite(storage->data, 1, storage->size, file) != storage->size) {
        ESP_LOGE(TAG_R, "write_file, write error");
    }

    fclose(file);
}

bool redundant_storage_read_copy(const char *filename, redundant_storage *storage) {
    FILE *file = fopen(filename, "rb");
    bool ready = read_file(file, storage);
    if (file != NULL) {
        fclose(file);
    }
    return ready;
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
    redundant_storage storage_0;
    bool storage_0_ready = redundant_storage_read_copy(filename_0, &storage_0);
    if (!storage_0_ready) {
        memset(&storage_0, 0, sizeof(storage_0));
    }
    close_storage(partition_0);

    open_storage(partition_1, path_1);
    snprintf(filename_1, sizeof(filename_1), "%s/%s", path_1, name);
    redundant_storage storage_1;
    bool storage_1_ready = redundant_storage_read_copy(filename_1, &storage_1);
    if (!storage_1_ready) {
        memset(&storage_1, 0, sizeof(storage_1));
    }
    close_storage(partition_1);

    if (storage_0_ready) {
        if (!storage_1_ready) {
            open_storage(partition_1, path_1);
            redundant_storage_write_copy(filename_1, &storage_0);
            close_storage(partition_1);
        } else {
            delete[] storage_1.data;
        }
    }

    if (!storage_0_ready && storage_1_ready) {
        open_storage(partition_0, path_0);
        redundant_storage_write_copy(filename_0, &storage_1);
        close_storage(partition_0);
        storage_0 = storage_1;
    }

    return storage_0;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage *storage) {

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path_0, name);

    open_storage(partition_0, path_0);
    redundant_storage_write_copy(filename, storage);
    close_storage(partition_0);

    snprintf(filename, sizeof(filename), "%s/%s", path_1, name);
    open_storage(partition_1, path_1);
    redundant_storage_write_copy(filename, storage);
    close_storage(partition_1);
}

void redundant_storage_delete(const char *partition_0,
                              const char *path_0,
                              const char *partition_1,
                              const char *path_1,
                              const char *name) {

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path_0, name);

    open_storage(partition_0, path_0);
    if (remove(filename) != 0) {
        ESP_LOGE(TAG_R, "Error deleting file '%s'", filename);
    }
    close_storage(partition_0);

    snprintf(filename, sizeof(filename), "%s/%s", path_1, name);
    open_storage(partition_1, path_1);
    if (remove(filename) != 0) {
        ESP_LOGE(TAG_R, "Error deleting file '%s'", filename);
    }
    close_storage(partition_1);
}