#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t value;
    bool required;
} ControllerIOValue;

typedef struct {
    ControllerIOValue AI;
    ControllerIOValue DI;
    ControllerIOValue O1;
    ControllerIOValue O2;
    uint8_t V1;
    uint8_t V2;
    uint8_t V3;
    uint8_t V4;
} ControllerIOValues;

class Controller {
  protected:
    static bool runned;
    static EventGroupHandle_t gpio_events;
    static TaskHandle_t process_task_handle;
    static uint8_t var1;
    static uint8_t var2;
    static uint8_t var3;
    static uint8_t var4;
    static Ladder *ladder;
    static ProcessWakeupService *processWakeupService;

    static std::recursive_mutex lock_io_values_mutex;
    static ControllerIOValues cached_io_values;

  public:
    static void Start(EventGroupHandle_t gpio_events);
    static void Stop();
    static bool SampleIOValues();
    static ControllerIOValues &GetIOValues();

    static void ProcessTask(void *parm);
    static void RenderTask(void *parm);

    static ControllerDI DI;
    static ControllerAI AI;

    static uint8_t GetO1RelativeValue();
    static uint8_t GetO2RelativeValue();
    static uint8_t GetV1RelativeValue();
    static uint8_t GetV2RelativeValue();
    static uint8_t GetV3RelativeValue();
    static uint8_t GetV4RelativeValue();

    static void SetO1RelativeValue(uint8_t value);
    static void SetO2RelativeValue(uint8_t value);
    static void SetV1RelativeValue(uint8_t value);
    static void SetV2RelativeValue(uint8_t value);
    static void SetV3RelativeValue(uint8_t value);
    static void SetV4RelativeValue(uint8_t value);

    static bool RequestWakeupMs(void *id, uint32_t delay_ms);
    static void RemoveRequestWakeupMs(void *id);
    static void RemoveExpiredWakeupRequests();
};
