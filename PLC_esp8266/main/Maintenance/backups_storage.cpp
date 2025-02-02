
#include "backups_storage.h"
#include "crc32.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    uint32_t crc;
} backups_storage_header;

static const char *TAG_B = "backups_storage";
static const char *partition = backups_storage_name;
const char *path = backups_path;

static bool read_file(FILE *file, backups_storage *storage) {
    if (file == NULL) {
        ESP_LOGE(TAG_B, "check_file, file not exists");
        return false;
    }

    struct stat st;

    if (fstat(fileno(file), &st) != 0 || (size_t)st.st_size < sizeof(backups_storage_header)) {
        ESP_LOGE(TAG_B, "check_file, file size wrong");
        return false;
    }

    backups_storage_header header;
    if (fread(&header, 1, sizeof(backups_storage_header), file) != sizeof(backups_storage_header)) {
        ESP_LOGE(TAG_B, "check_file, read header error");
        return false;
    }

    if (fread(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)) {
        ESP_LOGE(TAG_B, "check_file, read version error");
        return storage;
    }
    uint32_t crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));

    storage->size = st.st_size - sizeof(backups_storage_header) - sizeof(storage->version);
    storage->data = new uint8_t[storage->size];

    if (fread(storage->data, 1, storage->size, file) != storage->size) {
        ESP_LOGE(TAG_B, "check_file, read data error");
        delete[] storage->data;
        return false;
    }

    if (header.crc != calc_crc32(crc, storage->data, storage->size)) {
        ESP_LOGW(TAG_B, "check_file, wrong crc\r");
        delete[] storage->data;
        return false;
    }

    return true;
}

static void write_file(const char *path, backups_storage *storage) {
    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        ESP_LOGE(TAG_B, "write_file, file open error");
        return;
    }

    backups_storage_header header;

    header.crc = calc_crc32(CRC32_INIT, &storage->version, sizeof(storage->version));
    header.crc = calc_crc32(header.crc, storage->data, storage->size);

    if (fwrite(&header, 1, sizeof(backups_storage_header), file) != sizeof(backups_storage_header)
        || fwrite(&storage->version, 1, sizeof(storage->version), file) != sizeof(storage->version)
        || fwrite(storage->data, 1, storage->size, file) != storage->size) {
        ESP_LOGE(TAG_B, "write_file, write error");
    }

    fclose(file);
}

bool backups_storage_load(const char *name, backups_storage *storage) {
    ESP_LOGI(TAG_B, "backups_storage load path:'%s', name:'%s'", path, name);

    char filename[256];

    open_storage(partition, path);
    snprintf(filename, sizeof(filename), "%s/%s", path, name);
    FILE *file = fopen(filename, "rb");
    bool ready = read_file(file, storage);
    if (file != NULL) {
        fclose(file);
    }
    close_storage(partition);
    return ready;
}

void backups_storage_store(const char *name, backups_storage *storage) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s", path, name);

    open_storage(partition, path);
    write_file(filename, storage);
    close_storage(partition);
}
