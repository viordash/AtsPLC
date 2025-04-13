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

#include "DataMigrations/MigrateSettings.h"
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
  public:
    const static int YearOffset = 1900;
    DatetimeService();
    ~DatetimeService();

    int GetCurrentSecond();
    int GetCurrentMinute();
    int GetCurrentHour();
    int GetCurrentDay();
    int GetCurrentWeekday();
    int GetCurrentMonth();
    int GetCurrentYear();

    void Set(CurrentSettings::datetime_settings *datetime);
};
