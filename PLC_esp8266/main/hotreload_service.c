#include "hotreload_service.h"
#include "crc32.h"
#include "esp8266/eagle_soc.h"
#include "esp_log.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG_hotreload = "hotreload";

static const uint32_t MAGIC = 0xDE4572BB;

typedef uint32_t t_crc;

typedef struct {
    hotreload_data data;
    union {
        uint32_t magic;
        uint8_t dummy[256 - sizeof(hotreload_data) - sizeof(t_crc)];
    };
    t_crc crc;
} rtc_hotreload_data;

volatile rtc_hotreload_data *_rtc_hotreload_data = (volatile rtc_hotreload_data *)RTC_USER_BASE;
hotreload_data *hotreload = NULL;

_Static_assert(sizeof(hotreload->is_hotstart) == 4, "sizeof(is_hotstart)");
_Static_assert(sizeof(hotreload->restart_count) == 4, "sizeof(restart_count)");
_Static_assert(sizeof(hotreload->view_top_index) == 4, "sizeof(view_top_index)");
_Static_assert(sizeof(hotreload->selected_network) == 4, "sizeof(selected_network)");

void init_hotreload() {
    hotreload->is_hotstart = false;
    hotreload->restart_count = 0;
    hotreload->view_top_index = 0;
    hotreload->selected_network = -1;
}

void load_hotreload() {
    hotreload = (hotreload_data *)&_rtc_hotreload_data->data;
    if (_rtc_hotreload_data->magic != MAGIC) {
        ESP_LOGW(TAG_hotreload, "try_load_hotreload, incorrect magic");
        init_hotreload();
        return;
    }

    t_crc crc = calc_crc32(CRC32_INIT,
                           (const void *)&_rtc_hotreload_data->data,
                           sizeof(_rtc_hotreload_data->data));
    crc = calc_crc32(crc,
                     (const void *)&_rtc_hotreload_data->dummy,
                     sizeof(_rtc_hotreload_data->dummy));

    if (_rtc_hotreload_data->crc != crc) {
        ESP_LOGW(TAG_hotreload, "try_load_hotreload, wrong crc");
        init_hotreload();
        return;
    }
    hotreload->is_hotstart = true;
}

void store_hotreload() {
    _rtc_hotreload_data->magic = MAGIC;
    t_crc crc = calc_crc32(CRC32_INIT, (const void *)hotreload, sizeof(*hotreload));
    crc = calc_crc32(crc,
                     (const void *)&_rtc_hotreload_data->dummy,
                     sizeof(_rtc_hotreload_data->dummy));
    _rtc_hotreload_data->crc = crc;
}