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
#include "LogicProgram/MapIO.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class RenderingService;
class WiFiService;
class Ladder;
class Controller {
  protected:
    static bool runned;
    static bool force_process_loop;
    static EventGroupHandle_t gpio_events;
    static TaskHandle_t process_task_handle;
    static Ladder *ladder;
    static ProcessWakeupService *processWakeupService;
    static WiFiService *wifi_service;
    static RenderingService *rendering_service;

  public:
    static const int WAKEUP_PROCESS_TASK = BIT15;

    static void Start(EventGroupHandle_t gpio_events,
                      WiFiService *wifi_service,
                      RenderingService *rendering_service);
    static void Stop();
    static void FetchIOValues();
    static void CommitChanges();

    static void ProcessTask(void *parm);

    static ControllerDI DI;
    static ControllerAI AI;
    static ControllerDO O1;
    static ControllerDO O2;
    static ControllerVariable V1;
    static ControllerVariable V2;
    static ControllerVariable V3;
    static ControllerVariable V4;

    static bool RequestWakeupMs(void *id, uint32_t delay_ms, ProcessWakeupRequestPriority priority);
    static void RemoveRequestWakeupMs(void *id);
    static void RemoveExpiredWakeupRequests();

    static void BindVariableToSecureWiFi(const MapIO io_adr,
                                         const char *ssid,
                                         const char *password,
                                         const char *mac);
    static void BindVariableToInsecureWiFi(const MapIO io_adr, const char *ssid);
    static void BindVariableToStaWiFi(const MapIO io_adr);

    static void UnbindVariable(const MapIO io_adr);

    static void WakeupProcessTask();

    static uint8_t ConnectToWiFiStation();
    static void DisconnectFromWiFiStation();
};
