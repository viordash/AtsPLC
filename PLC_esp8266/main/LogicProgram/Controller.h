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
#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/Ladder.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Controller {
  protected:
    static bool runned;
    static EventGroupHandle_t gpio_events;
    static TaskHandle_t process_task_handle;
    static Ladder *ladder;
    static ProcessWakeupService *processWakeupService;

  public:
    static void Start(EventGroupHandle_t gpio_events);
    static void Stop();
    static bool SampleIOValues();

    static void ProcessTask(void *parm);
    static void RenderTask(void *parm);

    static ControllerDI DI;
    static ControllerAI AI;
    static ControllerDO O1;
    static ControllerDO O2;
    static ControllerVariable V1;
    static ControllerVariable V2;
    static ControllerVariable V3;
    static ControllerVariable V4;

    static bool RequestWakeupMs(void *id, uint32_t delay_ms);
    static void RemoveRequestWakeupMs(void *id);
    static void RemoveExpiredWakeupRequests();

    static void BindVariableToWiFi(const MapIO io_adr, const char *ssid);
    static void UnbindVariable(const MapIO io_adr);
};
