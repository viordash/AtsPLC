#include "hotreload_service.h"
#include "crc32.h"
#include "esp8266/eagle_soc.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hotreload";

static const uint32_t MAGIC = 0xDE4572BB;

typedef uint32_t t_crc;

typedef struct {
    hotreload data;
    union {
        uint32_t magic;
        uint8_t dummy[256 - sizeof(hotreload) - sizeof(t_crc)];
    };
    t_crc crc;
} hotreload_data;

volatile hotreload_data *_hotreload_data = (volatile hotreload_data *)RTC_USER_BASE;

bool try_load_hotreload(hotreload *data) {
    if (_hotreload_data->magic != MAGIC) {
        ESP_LOGW(TAG, "try_load_hotreload, incorrect magic\r\n");
        return false;
    }

    t_crc crc =
        calc_crc32(CRC32_INIT, (const void *)&_hotreload_data->data, sizeof(_hotreload_data->data));
    crc = calc_crc32(crc, (const void *)&_hotreload_data->dummy, sizeof(_hotreload_data->dummy));

    if (_hotreload_data->crc != crc) {
        ESP_LOGW(TAG, "try_load_hotreload, wrong crc\r\n");
        return false;
    }
    *data = _hotreload_data->data;
    return true;
}

void store_hotreload(hotreload *data) {
    _hotreload_data->magic = MAGIC;
    t_crc crc = calc_crc32(CRC32_INIT, (const void *)data, sizeof(*data));
    crc = calc_crc32(crc, (const void *)&_hotreload_data->dummy, sizeof(_hotreload_data->dummy));
    _hotreload_data->crc = crc;
    _hotreload_data->data = *data;
}