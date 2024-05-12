#include "hotreload_service.h"
#include "crc32.h"
#include "esp8266/eagle_soc.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hotreload";

typedef struct {
    hotreload data;
    uint32_t crc;
} hotreload_data;

volatile hotreload_data *_hotreload_data = (volatile hotreload_data *)RTC_USER_BASE;

bool try_load_hotreload(hotreload *data) {
    if (_hotreload_data->crc
        != calc_crc32(CRC32_INIT,
                      (const void *)&_hotreload_data->data,
                      sizeof(_hotreload_data->data))) {

        ESP_LOGW(TAG, "try_load_hotreload, wrong crc\r\n");
        return false;
    }
    memcpy((void *)data, (const void *)&_hotreload_data->data, sizeof(*data));
    return true;
}

void store_hotreload(hotreload *data) {
    _hotreload_data->crc = calc_crc32(CRC32_INIT, data, sizeof(*data));
    memcpy((void *)&_hotreload_data->data, (const void *)data, sizeof(_hotreload_data->data));
}