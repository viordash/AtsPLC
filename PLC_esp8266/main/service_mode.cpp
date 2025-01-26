
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
}

#include "Display/Common.h"
#include "Display/display.h"
#include "esp_err.h"
#include "esp_log.h"
#include "service_mode.h"
#include "settings.h"
#include "smartconfig_service.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_service_mode = "service_mode";

enum ServiceMode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

static void render_service_main_menu(ServiceMode mode) {
    char buffer[64];
    uint8_t x = 1;
    uint8_t y = 1;
    uint8_t height = get_text_f6X12_height();
    uint8_t *fb = begin_render();

    sprintf(buffer, "v%08X.%s", DEVICE_SETTINGS_VERSION, __TIME__);
    ESP_ERROR_CHECK(draw_text_f5X7(fb, x, y, buffer) <= 0);

    uint8_t marker_x = x;
    uint8_t marker_y = y;
    switch (mode) {
        case ServiceMode::sm_SmartConfig:
            marker_y += height * 1;
            break;
        case ServiceMode::sm_BackupLogic:
            marker_y += height * 2;
            break;
        case ServiceMode::sm_RestoreLogic:
            marker_y += height * 3;
            break;
        case ServiceMode::sm_ResetToDefault:
            marker_y += height * 4;
            break;
    }
    int marker_width = draw_text_f6X12(fb, marker_x, marker_y, "\x01");
    ESP_ERROR_CHECK(marker_width <= 0);
    x += marker_width;

    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 1, "  Smart config") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 2, "  Backup logic") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 3, "  Restore logic") <= 0);
    ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 4, "  Reset to default") <= 0);

    end_render(fb);
}

void run_service_mode() {
    render_service_main_menu(ServiceMode::sm_ResetToDefault);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    render_service_main_menu(ServiceMode::sm_RestoreLogic);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    render_service_main_menu(ServiceMode::sm_BackupLogic);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    render_service_main_menu(ServiceMode::sm_SmartConfig);

    start_smartconfig();
    while (smartconfig_is_runned()) {
        ESP_LOGI(TAG_service_mode, "wait smartconfig...");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
