
#include "restart_counter.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include <string.h>

static const char *TAG_restart_counter = "restart_counter";

void hot_restart_counter() {
    hotreload hotreload_data;
    if (!try_load_hotreload(&hotreload_data)) {
        memset(&hotreload_data, 0, sizeof(hotreload_data));
    }
    hotreload_data.restart_count++;
    store_hotreload(&hotreload_data);

    ESP_LOGI(TAG_restart_counter, "count:%u", hotreload_data.restart_count);
}