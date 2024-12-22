#include "CppUTestExt/MockSupport_c.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_event_legacy.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdlib.h>

const char *WIFI_EVENT = "wifi_event";
const char *IP_EVENT = "ip_event";
const char *ETH_EVENT = "eth_event";

const char *esp_err_to_name(esp_err_t code) {
    (void)code;
    return "UNIT_TESTS_WRAPPER";
}

esp_err_t gpio_config(const gpio_config_t *config) {
    return mock_c()
        ->actualCall("gpio_config")
        ->withConstPointerParameters("config", config)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t adc_init(adc_config_t *config) {
    return mock_c()
        ->actualCall("adc_init")
        ->withConstPointerParameters("config", config)
        ->returnIntValueOrDefault(ESP_OK);
}

int gpio_get_level(gpio_num_t gpio_num) {
    char buffer[32];
    sprintf(buffer, "%d", gpio_num);
    return mock_scope_c(buffer)->actualCall("gpio_get_level")->returnIntValueOrDefault(-1);
}

esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) {
    char buffer[32];
    sprintf(buffer, "%d", gpio_num);
    return mock_scope_c(buffer)
        ->actualCall("gpio_set_level")
        ->withUnsignedIntParameters("level", level)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t adc_read(uint16_t *data) {
    esp_err_t err = mock_c()
                        ->actualCall("adc_read")
                        ->withOutputParameter("adc", data)
                        ->returnIntValueOrDefault(ESP_OK);
    return err;
}

/*

*/
esp_err_t esp_vfs_spiffs_register(const esp_vfs_spiffs_conf_t *conf) {
    return mock_scope_c(conf->partition_label)
        ->actualCall("esp_vfs_spiffs_register")
        ->withConstPointerParameters("conf", conf)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_vfs_spiffs_unregister(const char *partition_label) {
    return mock_scope_c(partition_label)
        ->actualCall("esp_vfs_spiffs_unregister")
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t gpio_install_isr_service(int no_use) {
    (void)no_use;
    return mock_c()->actualCall("gpio_install_isr_service")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void *args) {
    (void)args;
    char buffer[32];
    sprintf(buffer, "%d", gpio_num);

    gpio_isr_t *p = (gpio_isr_t *)mock_scope_c(buffer)->getData("isr_handler").value.pointerValue;
    if (p != NULL) {
        *p = isr_handler;
    }
    return mock_scope_c(buffer)
        ->actualCall("gpio_isr_handler_add")
        ->returnIntValueOrDefault(ESP_OK);
}

EventGroupHandle_t xEventGroupCreate(void) {
    return mock_c()->actualCall("xEventGroupCreate")->returnPointerValueOrDefault(NULL);
}
void vEventGroupDelete(EventGroupHandle_t xEventGroup) {
    mock_c()->actualCall("vEventGroupDelete")->withPointerParameters("xEventGroup", xEventGroup);
}

BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     BaseType_t *pxHigherPriorityTaskWoken) {
    (void)pxHigherPriorityTaskWoken;
    char buffer[32];
    sprintf(buffer, "0x%08X", uxBitsToSet);

    return mock_scope_c(buffer)
        ->actualCall("xEventGroupSetBitsFromISR")
        ->withPointerParameters("xEventGroup", xEventGroup)
        ->returnIntValueOrDefault(pdTRUE);
}

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet) {
    char buffer[32];
    sprintf(buffer, "0x%08X", uxBitsToSet);

    return mock_scope_c(buffer)
        ->actualCall("xEventGroupSetBits")
        ->withPointerParameters("xEventGroup", xEventGroup)
        ->returnIntValueOrDefault(pdTRUE);
}

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear) {
    char buffer[32];
    sprintf(buffer, "0x%08X", uxBitsToClear);

    return mock_scope_c(buffer)
        ->actualCall("xEventGroupClearBits")
        ->withPointerParameters("xEventGroup", xEventGroup)
        ->returnIntValueOrDefault(pdTRUE);
}

EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToWaitFor,
                                const BaseType_t xClearOnExit,
                                const BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait) {

    return mock_c()
        ->actualCall("xEventGroupWaitBits")
        ->withPointerParameters("xEventGroup", xEventGroup)
        ->withUnsignedIntParameters("uxBitsToWaitFor", uxBitsToWaitFor)
        ->withIntParameters("xClearOnExit", xClearOnExit)
        ->withIntParameters("xWaitForAllBits", xWaitForAllBits)
        ->withUnsignedIntParameters("xTicksToWait", xTicksToWait)
        ->returnUnsignedIntValueOrDefault(0);
}

TickType_t xTaskGetTickCount(void) {
    TickType_t ticks;
    mock_c()->actualCall("xTaskGetTickCount")->withOutputParameter("ticks", &ticks);
    return ticks;
}

void portYIELD_FROM_ISR(void) {
}

int64_t esp_timer_get_time(void) {
    uint64_t os_us;
    mock_c()->actualCall("esp_timer_get_time")->withOutputParameter("os_us", &os_us);
    return os_us;
}

void vTaskDelay(const TickType_t xTicksToDelay) {
    mock_c()->actualCall("vTaskDelay")->withUnsignedIntParameters("xTicksToDelay", xTicksToDelay);
}

void vTaskDelayUntil(TickType_t *const pxPreviousWakeTime, const TickType_t xTimeIncrement) {
    mock_c()
        ->actualCall("vTaskDelayUntil")
        ->withConstPointerParameters("pxPreviousWakeTime", pxPreviousWakeTime)
        ->withUnsignedIntParameters("xTimeIncrement", xTimeIncrement);
}

BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char *const
        pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth,
    void *const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask) {

    (void)pxTaskCode;
    (void)usStackDepth;
    (void)pvParameters;
    (void)uxPriority;
    (void)pxCreatedTask;

    return mock_c()
        ->actualCall("xTaskCreate")
        ->withStringParameters("pcName", pcName)
        ->returnIntValueOrDefault(pdPASS);
}

void vTaskDelete(TaskHandle_t xTaskToDelete) {
    mock_c()->actualCall("vTaskDelete")->withPointerParameters("xTaskToDelete", xTaskToDelete);
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait) {
    return mock_c()
        ->actualCall("xTaskNotifyWait")
        ->withUnsignedIntParameters("ulBitsToClearOnEntry", ulBitsToClearOnEntry)
        ->withUnsignedIntParameters("ulBitsToClearOnExit", ulBitsToClearOnExit)
        ->withPointerParameters("pulNotificationValue", pulNotificationValue)
        ->withUnsignedIntParameters("xTicksToWait", xTicksToWait)
        ->returnIntValueOrDefault(pdPASS);
}

BaseType_t xTaskGenericNotify(TaskHandle_t xTaskToNotify,
                              uint32_t ulValue,
                              eNotifyAction eAction,
                              uint32_t *pulPreviousNotificationValue) {
    return mock_c()
        ->actualCall("xTaskGenericNotify")
        ->withPointerParameters("xTaskToNotify", xTaskToNotify)
        ->withUnsignedIntParameters("ulValue", ulValue)
        ->withIntParameters("eAction", (int)eAction)
        ->withPointerParameters("pulPreviousNotificationValue", pulPreviousNotificationValue)
        ->returnIntValueOrDefault(pdPASS);
}

esp_err_t esp_event_send(system_event_t *event) {
    return mock_c()
        ->actualCall("esp_event_send")
        ->withPointerParameters("event", event)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
    return mock_c()
        ->actualCall("esp_wifi_init")
        ->withConstPointerParameters("config", config)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_event_handler_register(esp_event_base_t event_base,
                                     int32_t event_id,
                                     esp_event_handler_t event_handler,
                                     void *event_handler_arg) {
    (void)event_handler;
    (void)event_handler_arg;
    return mock_c()
        ->actualCall("esp_event_handler_register")
        ->withConstPointerParameters("event_base", event_base)
        ->withIntParameters("event_id", event_id)
        ->returnIntValueOrDefault(ESP_OK);
}
esp_err_t esp_event_handler_unregister(esp_event_base_t event_base,
                                       int32_t event_id,
                                       esp_event_handler_t event_handler) {
    (void)event_handler;
    return mock_c()
        ->actualCall("esp_event_handler_unregister")
        ->withConstPointerParameters("event_base", event_base)
        ->withIntParameters("event_id", event_id)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_start(void) {
    return mock_c()->actualCall("esp_wifi_start")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_connect(void) {
    return mock_c()->actualCall("esp_wifi_connect")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_disconnect(void) {
    return mock_c()->actualCall("esp_wifi_disconnect")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_stop(void) {
    return mock_c()->actualCall("esp_wifi_stop")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_deinit(void) {
    return mock_c()->actualCall("esp_wifi_deinit")->returnIntValueOrDefault(ESP_OK);
}

esp_err_t tcpip_adapter_clear_default_wifi_handlers() {
    return mock_c()
        ->actualCall("tcpip_adapter_clear_default_wifi_handlers")
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_set_mode(wifi_mode_t mode) {
    return mock_c()
        ->actualCall("esp_wifi_set_mode")
        ->withIntParameters("mode", mode)
        ->returnIntValueOrDefault(ESP_OK);
}

esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf) {
    return mock_c()
        ->actualCall("esp_wifi_set_config")
        ->withIntParameters("interface", interface)
        ->withPointerParameters("conf", conf)
        ->returnIntValueOrDefault(ESP_OK);
}