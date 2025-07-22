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

#include "Datetime/Datetime.h"
#include "LogicProgram/ControllerAI.h"
#include "LogicProgram/ControllerDI.h"
#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/ControllerVariable.h"
#include "LogicProgram/LogicItemState.h"
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
class DatetimeService;
class Controller {
  protected:
    static bool runned;
    static bool force_process_loop;
    static EventGroupHandle_t gpio_events;
    static TaskHandle_t process_task_handle;
    static ProcessWakeupService *processWakeupService;
    static WiFiService *wifi_service;
    static RenderingService *rendering_service;
    static DatetimeService *datetime_service;
    static LogicItemState network_continuation;

  public:
    static const int WAKEUP_PROCESS_TASK = BIT15;

    static void Start(EventGroupHandle_t gpio_events,
                      WiFiService *wifi_service,
                      RenderingService *rendering_service,
                      DatetimeService *datetime_service);
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
    static uint32_t GetWakeupTicks();

    static void BindVariableToSecureWiFi(const MapIO io_adr,
                                         const char *ssid,
                                         const char *password,
                                         const char *mac);
    static void BindVariableToInsecureWiFi(const MapIO io_adr, const char *ssid);
    static void BindVariableToStaWiFi(const MapIO io_adr);
    static void BindVariableToToDateTime(const MapIO io_adr, DatetimePart datetime_part);
    static void UnbindVariable(const MapIO io_adr);

    static void WakeupProcessTask();

    static uint8_t ConnectToWiFiStation();
    static void DisconnectFromWiFiStation();

    static bool ManualSetSystemDatetime(Datetime *dt);
    static void GetSystemDatetime(Datetime *dt);
    static void RestartSntp();
    static void StoreSystemDatetime();
    static void SetNetworkContinuation(LogicItemState state);
    static LogicItemState GetNetworkContinuation();

    static void UpdateUIViewTop(int32_t view_top_index);
    static void UpdateUISelected(int32_t selected_network);
    static int32_t GetLastUpdatedUISelected();
};
