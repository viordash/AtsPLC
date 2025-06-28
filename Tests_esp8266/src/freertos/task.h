#pragma once

#define PRIVILEGED_FUNCTION

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define tskKERNEL_VERSION_NUMBER "V10.0.1"
#define tskKERNEL_VERSION_MAJOR 10
#define tskKERNEL_VERSION_MINOR 0
#define tskKERNEL_VERSION_BUILD 1

/**
 * task. h
 *
 * Type by which tasks are referenced.  For example, a call to xTaskCreate
 * returns (via a pointer parameter) an TaskHandle_t variable that can then
 * be used as a parameter to vTaskDelete to delete the task.
 *
 * \defgroup TaskHandle_t TaskHandle_t
 * \ingroup Tasks
 */
typedef void *TaskHandle_t;

/*
 * Defines the prototype to which the application task hook function must
 * conform.
 */
typedef BaseType_t (*TaskHookFunction_t)(void *);

/* Task states returned by eTaskGetState. */
typedef enum {
    eRunning = 0, /* A task is querying the state of itself, so must be running. */
    eReady,       /* The task being queried is in a read or pending ready list. */
    eBlocked,     /* The task being queried is in the Blocked state. */
    eSuspended, /* The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
    eDeleted,   /* The task being queried has been deleted, but its TCB has not yet been freed. */
    eInvalid    /* Used as an 'invalid state' value. */
} eTaskState;

/* Actions that can be performed when vTaskNotify() is called. */
typedef enum {
    eNoAction,              /* Notify the task without updating its notify value. */
    eSetBits,               /* Set bits in the task's notification value. */
    eIncrement,             /* Increment the task's notification value. */
    eSetValueWithOverwrite, /* Set the task's notification value to a specific value even if the previous value has not yet been read by the task. */
    eSetValueWithoutOverwrite /* Set the task's notification value if the previous value has been read by the task. */
} eNotifyAction;

/* Used with the uxTaskGetSystemState() function to return the state of each task
in the system. */
typedef struct xTASK_STATUS {
    TaskHandle_t
        xHandle; /* The handle of the task to which the rest of the information in the structure relates. */
    const char *pcTaskName;
    /* A pointer to the task's name.  This value will be invalid if the task was deleted since the structure was populated! */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    UBaseType_t xTaskNumber; /* A number unique to the task. */
    eTaskState
        eCurrentState; /* The state in which the task existed when the structure was populated. */
    UBaseType_t
        uxCurrentPriority; /* The priority at which the task was running (may be inherited) when the structure was populated. */
    UBaseType_t
        uxBasePriority; /* The priority to which the task will return if the task's current priority has been inherited to avoid unbounded priority inversion when obtaining a mutex.  Only valid if configUSE_MUTEXES is defined as 1 in FreeRTOSConfig.h. */
    uint32_t
        ulRunTimeCounter; /* The total run time allocated to the task so far, as defined by the run time stats clock.  See http://www.freertos.org/rtos-run-time-stats.html.  Only valid when configGENERATE_RUN_TIME_STATS is defined as 1 in FreeRTOSConfig.h. */
    StackType_t *pxStackBase; /* Points to the lowest address of the task's stack area. */
    uint16_t
        usStackHighWaterMark; /* The minimum amount of stack space that has remained for the task since the task was created.  The closer this value is to zero the closer the task has come to overflowing its stack. */
} TaskStatus_t;

/**
 * task. h
 * <pre>eTaskState eTaskGetState( TaskHandle_t xTask );</pre>
 *
 * INCLUDE_eTaskGetState must be defined as 1 for this function to be available.
 * See the configuration section for more information.
 *
 * Obtain the state of any task.  States are encoded by the eTaskState
 * enumerated type.
 *
 * @param xTask Handle of the task to be queried.
 *
 * @return The state of xTask at the time the function was called.  Note the
 * state of the task might change between the function being called, and the
 * functions return value being tested by the calling task.
 */
eTaskState eTaskGetState(TaskHandle_t xTask) PRIVILEGED_FUNCTION;

/**
 * task. h
 * <pre>void vTaskGetInfo( TaskHandle_t xTask, TaskStatus_t *pxTaskStatus, BaseType_t xGetFreeStackSpace, eTaskState eState );</pre>
 *
 * configUSE_TRACE_FACILITY must be defined as 1 for this function to be
 * available.  See the configuration section for more information.
 *
 * Populates a TaskStatus_t structure with information about a task.
 *
 * @param xTask Handle of the task being queried.  If xTask is NULL then
 * information will be returned about the calling task.
 *
 * @param pxTaskStatus A pointer to the TaskStatus_t structure that will be
 * filled with information about the task referenced by the handle passed using
 * the xTask parameter.
 *
 * @xGetFreeStackSpace The TaskStatus_t structure contains a member to report
 * the stack high water mark of the task being queried.  Calculating the stack
 * high water mark takes a relatively long time, and can make the system
 * temporarily unresponsive - so the xGetFreeStackSpace parameter is provided to
 * allow the high water mark checking to be skipped.  The high watermark value
 * will only be written to the TaskStatus_t structure if xGetFreeStackSpace is
 * not set to pdFALSE;
 *
 * @param eState The TaskStatus_t structure contains a member to report the
 * state of the task being queried.  Obtaining the task state is not as fast as
 * a simple assignment - so the eState parameter is provided to allow the state
 * information to be omitted from the TaskStatus_t structure.  To obtain state
 * information then set eState to eInvalid - otherwise the value passed in
 * eState will be reported as the task state in the TaskStatus_t structure.
 *
 * Example usage:
   <pre>
 void vAFunction( void )
 {
 TaskHandle_t xHandle;
 TaskStatus_t xTaskDetails;

    // Obtain the handle of a task from its name.
    xHandle = xTaskGetHandle( "Task_Name" );

    // Check the handle is not NULL.
    configASSERT( xHandle );

    // Use the handle to obtain further information about the task.
    vTaskGetInfo( xHandle,
                  &xTaskDetails,
                  pdTRUE, // Include the high water mark in xTaskDetails.
                  eInvalid ); // Include the task state in xTaskDetails.
 }
   </pre>
 * \defgroup vTaskGetInfo vTaskGetInfo
 * \ingroup TaskCtrl
 */
void vTaskGetInfo(TaskHandle_t xTask,
                  TaskStatus_t *pxTaskStatus,
                  BaseType_t xGetFreeStackSpace,
                  eTaskState eState) PRIVILEGED_FUNCTION;

TickType_t xTaskGetTickCount(void) PRIVILEGED_FUNCTION;

#define tskIDLE_PRIORITY ((UBaseType_t)0U)

void vTaskDelay(const TickType_t xTicksToDelay) PRIVILEGED_FUNCTION;

void vTaskDelayUntil(TickType_t *const pxPreviousWakeTime,
                     const TickType_t xTimeIncrement) PRIVILEGED_FUNCTION;

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

TaskHandle_t xTaskGetCurrentTaskHandle(void) PRIVILEGED_FUNCTION;

#ifdef __cplusplus
}
#endif