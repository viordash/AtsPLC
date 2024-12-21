#pragma once

#define PRIVILEGED_FUNCTION

#include "FreeRTOS.h"

/* Actions that can be performed when vTaskNotify() is called. */
typedef enum {
    eNoAction,              /* Notify the task without updating its notify value. */
    eSetBits,               /* Set bits in the task's notification value. */
    eIncrement,             /* Increment the task's notification value. */
    eSetValueWithOverwrite, /* Set the task's notification value to a specific value even if the previous value has not yet been read by the task. */
    eSetValueWithoutOverwrite /* Set the task's notification value if the previous value has been read by the task. */
} eNotifyAction;

#ifdef __cplusplus
extern "C" {
#endif

TickType_t xTaskGetTickCount(void) PRIVILEGED_FUNCTION;

#define tskIDLE_PRIORITY ((UBaseType_t)0U)

void vTaskDelay(const TickType_t xTicksToDelay) PRIVILEGED_FUNCTION;

void vTaskDelayUntil( TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement ) PRIVILEGED_FUNCTION;

BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char *const
        pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth,
    void *const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask) PRIVILEGED_FUNCTION;

void vTaskDelete(TaskHandle_t xTaskToDelete) PRIVILEGED_FUNCTION;

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait) PRIVILEGED_FUNCTION;

BaseType_t xTaskGenericNotify(TaskHandle_t xTaskToNotify,
                              uint32_t ulValue,
                              eNotifyAction eAction,
                              uint32_t *pulPreviousNotificationValue) PRIVILEGED_FUNCTION;

#define xTaskNotify(xTaskToNotify, ulValue, eAction)                                               \
    xTaskGenericNotify((xTaskToNotify), (ulValue), (eAction), NULL)

#ifdef __cplusplus
}
#endif