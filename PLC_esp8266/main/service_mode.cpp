
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
}

#include "esp_err.h"
#include "esp_log.h"
#include "service_mode.h"
#include "smartconfig_service.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_service_mode = "service_mode";

void run_service_mode() {
    start_smartconfig();
    while (smartconfig_is_runned()) {
        ESP_LOGI(TAG_service_mode, "wait smartconfig...");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
