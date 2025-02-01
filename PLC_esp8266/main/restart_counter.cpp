
#include "restart_counter.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include <string.h>

static const char *TAG_restart_counter = "restart_counter";

void hot_restart_counter() {
    hotreload->restart_count++;
    store_hotreload();

    ESP_LOGI(TAG_restart_counter, "count:%u", hotreload->restart_count);
}