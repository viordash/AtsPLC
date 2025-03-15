#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "Display/RenderingService.h"
#include "Display/display.h"
#include "LogicProgram/Controller.h"
#include "Maintenance/ServiceModeHandler.h"
#include "WiFi/WiFiService.h"
#include "buttons.h"
#include "crc32.h"
#include "driver/uart.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "hotreload_service.h"
#include "redundant_storage.h"
#include "restart_counter.h"
#include "settings.h"
#include "storage.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "main";

extern "C" void app_main();

extern CurrentSettings::device_settings settings;

static void system_init() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void app_main() {
    load_hotreload();

    if (hotreload->is_hotstart) {
        ESP_LOGI(TAG, "is hotstart");
    }

    EventGroupHandle_t gpio_events = gpio_init();

    load_settings();

    system_init();
    display_init();
    hot_restart_counter();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, , crc32:%u",
           chip_info.cores,
           calc_crc32(CRC32_INIT, &chip_info, sizeof(&chip_info)));

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n",
           spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    if (!hotreload->is_hotstart && up_button_pressed()) {
        ServiceModeHandler::Start(gpio_events);
    }

    WiFiService wifi_service;
    wifi_service.Start();
    RenderingService rendering_service;
    Controller::Start(gpio_events, &wifi_service, &rendering_service);

    uint32_t free_mem = esp_get_free_heap_size();
    printf("mem: %u\n", free_mem);
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        uint32_t curr_free_mem = esp_get_free_heap_size();
        int32_t dead_band_1perc = curr_free_mem / 100;
        int32_t diff = curr_free_mem - free_mem;
        if (diff > dead_band_1perc || diff < -dead_band_1perc) {
            printf("mem: %u\n", curr_free_mem);
            free_mem = curr_free_mem;
        }
    }

    Controller::Stop();
    wifi_service.Stop();
    store_settings();
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
