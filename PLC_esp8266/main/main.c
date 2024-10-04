/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Display/display.h"
#include "HttpServer/http_server.h"
#include "LogicProgram/process_engine.h"
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
#include "smartconfig_service.h"
#include "storage.h"
#include "sys_gpio.h"
#include "wifi_sta.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "main";

extern device_settings settings;

static void system_init() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

static void startup() {
    load_hotreload();

    if (hotreload->is_hotstart) {
        ESP_LOGI(TAG, "is hotstart");
    }

    EventGroupHandle_t gpio_events = gpio_init();

    load_settings();

    system_init();

    if (!hotreload->is_hotstart) {
        start_smartconfig();
    }

    display_init();
    start_process_engine(gpio_events);
}

void app_main() {
    startup();
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

    while (smartconfig_is_runned()) {
        printf("wait smartconfig...\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }

    bool has_wifi_sta_settings;
    SAFETY_SETTINGS(                                        //
        has_wifi_sta_settings = settings.wifi.ssid[0] != 0; //
    );
    if (has_wifi_sta_settings) {
        start_wifi_sta();
        start_http_server();
    }

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }

    stop_wifi_sta();
    stop_http_server();
    store_settings();
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
