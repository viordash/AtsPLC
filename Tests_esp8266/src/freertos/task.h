#define PRIVILEGED_FUNCTION

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

TickType_t xTaskGetTickCount(void) PRIVILEGED_FUNCTION;

void vTaskDelay(const TickType_t xTicksToDelay) PRIVILEGED_FUNCTION;

BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char *const
        pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth,
    void *const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask) PRIVILEGED_FUNCTION;

void vTaskDelete( TaskHandle_t xTaskToDelete ) PRIVILEGED_FUNCTION;

#ifdef __cplusplus
}
#endif