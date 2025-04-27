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
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

class DatetimeService {
  protected:
    TaskHandle_t task_handle;

    static void Task(void *parm);
    virtual void GetCurrent(timeval *tv);
    bool EnableSntp();
    void StartSntp();
    void StopSntp();
    bool SntpInProcess();

  public:
    static const int STORE_BIT = BIT1;
    static const int RESTART_SNTP_BIT = BIT2;

    const static int YearOffset = 1900;
    DatetimeService();
    virtual ~DatetimeService();

    void Start();

    void SntpStateChanged();
    void StoreSystemDatetime();

    int GetCurrentSecond();
    int GetCurrentMinute();
    int GetCurrentHour();
    int GetCurrentDay();
    int GetCurrentWeekday();
    int GetCurrentMonth();
    int GetCurrentYear();

    bool ManualSet(Datetime *dt);
    void Get(Datetime *dt);
};
