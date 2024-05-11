/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "crc32.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "gpio.h"
#include "hotreload_service.h"
#include "redundant_storage.h"
#include "storage.h"
#include <stdio.h>

static const char *TAG = "main";
static hotreload hotreload_data;

static const char *storage_0_partition = "storage_0";
static const char *storage_1_partition = "storage_1";
static const char *storage_0_path = "/storage_0";
static const char *storage_1_path = "/storage_1";

typedef struct {
    size_t size;
    uint32_t crc;
    uint32_t version;
} device_settings;

 static device_settings *settings;

void app_main() {
    uart_set_baudrate(UART_NUM_0, 921600);

    if (try_load_hotreload(&hotreload_data)) {
        ESP_LOGI(TAG, "hotreload, gpio:%u\n", hotreload_data.gpio);
    } else {
        hotreload_data.gpio = 0x00;
    }

    gpio_init(hotreload_data.gpio);

    redundant_storage settings_storage = redundant_storage_load(storage_0_partition,
                                                        storage_0_path,
                                                        storage_1_partition,
                                                        storage_1_path);

    settings = (device_settings *)settings_storage.data;

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

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        hotreload_data.gpio++;
    }
    store_hotreload(&hotreload_data);

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
