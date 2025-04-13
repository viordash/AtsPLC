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

#include "WiFiRequests.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

class DatetimeService {
  protected:
  public:
    DatetimeService();
    ~DatetimeService();

    int GetCurrentSecond();
    int GetCurrentMinute();
    int GetCurrentHour();
    int GetCurrentDay();
    int GetCurrentWeekday();
    int GetCurrentMonth();
    int GetCurrentYear();

    void SetCurrentSecond(int val);
    void SetCurrentMinute(int val);
    void SetCurrentHour(int val);
    void SetCurrentDay(int val);
    void SetCurrentMonth(int val);
    void SetCurrentYear(int val);
};
