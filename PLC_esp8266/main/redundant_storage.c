
#include "redundant_storage.h"
#include "crc32.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <sys/stat.h>

typedef struct {
    size_t size;
    uint32_t crc;
    uint32_t version;
} redundant_storage_header;

static const char *TAG = "redundant_storage";

// static bool file_exists(const char *path) {
//     struct stat st;
//     if (stat(path, &st) != 0) {
//         return false;
//     }
//     return st.st_size >= sizeof(redundant_storage_header);
// }

static bool check_file(const char *path, size_t size) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "check_file");
        return false;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
}

uint8_t *
open_redundant_storage(const char *path_0, const char *path_1, size_t size, uint32_t version) {
    ESP_LOGI(TAG, "Initializing redundant_storage path_0:'%s', path_1:'%s'", path_0, path_1);

    // if (file_exists(path_0))
    //     ESP_LOGI(TAG, "Opening file");

    //     ESP_LOGI(TAG, "File written");

    return NULL;
}

void store_redundant_storage(const char *path_0, const char *path_1, uint8_t *data) {
}

void close_redundant_storage(uint8_t *data) {
}

// bool StorageService::CalcCrc(Storage *storage, int len, uint32_t *pCrc) {
// 	uint8_t buffer[4096];
// 	while (len > 0) {
// 		int32_t readLen = len;
// 		if (readLen > sizeof(buffer)) {
// 			readLen = sizeof(buffer);
// 		}
// 		readLen = storage->Read((uint8_t *)buffer, readLen);
// 		if (readLen < 0) {
// 			return false;
// 		}
// 		*pCrc = crc32(*pCrc, (uint8_t *)buffer, readLen);
// 		len -= readLen;
// 	}
// 	return true;
// }

// bool StorageService::LoadHeader(Storage *storage, PTStorageHeader pStorageHeader) {
// 	uint32_t readedCrc;
// 	uint32_t crc = 0xFFFFFFFF;

// 	if (!storage->Read((uint8_t *)pStorageHeader, sizeof(TStorageHeader))) {
// 		return false;
// 	}
// 	if (pStorageHeader->Length < sizeof(TStorageHeader) || pStorageHeader->Length > MAX_STORAGE_SIZE) {
// 		return false;
// 	}
// 	if (pStorageHeader->Version != version) {
// 		return false;
// 	}
// 	readedCrc = pStorageHeader->Crc;
// 	pStorageHeader->Crc = 0;
// 	crc = crc32(crc, (uint8_t *)pStorageHeader, sizeof(TStorageHeader));

// 	return CalcCrc(storage, pStorageHeader->Length - sizeof(TStorageHeader), &crc) && readedCrc == crc;
// }