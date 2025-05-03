
#include "restart_counter.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include <string.h>

static const char *TAG_restart_counter = "restart_counter";

void hot_restart_counter() {
    SAFETY_HOTRELOAD({
        hotreload->restart_count++;
        store_hotreload();
    });

    ESP_LOGI(TAG_restart_counter, "count:%u", (unsigned int)hotreload->restart_count);
}