#include "DatetimeService.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "hotreload_service.h"
#include "settings.h"
#include "sys/time.h"
#include "sys_gpio.h"
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char *TAG_DatetimeService = "DatetimeService";
extern CurrentSettings::device_settings settings;

DatetimeService::DatetimeService() {
}

DatetimeService::~DatetimeService() {
}

int DatetimeService::GetCurrentSecond() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentSecond: %d", tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMinute: %d", tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentHour: %d", tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentDay: %d", tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentWeekday: %d", tm.tm_wday + 1);
    return tm.tm_wday + 1;
}

int DatetimeService::GetCurrentMonth() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMonth: %d", tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&tv.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentYear: %d", tm.tm_year);
    return tm.tm_year;
}

bool DatetimeService::ManualSet(Datetime *dt) {
    if (!ValidateDatetime(dt)) {
        ESP_LOGW(TAG_DatetimeService,
                 "ManualSet, invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
                 dt->year,
                 dt->month,
                 dt->day,
                 dt->hour,
                 dt->minute,
                 dt->second);
        return false;
    }

    SAFETY_HOTRELOAD({
        hotreload->current_datetime.year = dt->year;
        hotreload->current_datetime.month = dt->month;
        hotreload->current_datetime.day = dt->day;
        hotreload->current_datetime.hour = dt->hour;
        hotreload->current_datetime.minute = dt->minute;
        hotreload->current_datetime.second = dt->second;
        store_hotreload();
    });

    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    struct tm tm = *localtime(&tv.tv_sec);
    tm.tm_sec = dt->second;
    tm.tm_min = dt->minute;
    tm.tm_hour = dt->hour;
    tm.tm_mday = dt->day;
    tm.tm_mon = dt->month - 1;
    tm.tm_year = dt->year - DatetimeService::YearOffset;

    timeval new_tv = { mktime(&tm), tv.tv_usec };
    ESP_ERROR_CHECK(settimeofday(&new_tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    ESP_LOGI(TAG_DatetimeService,
             "ManualSet: %04d-%02d-%02d %02d:%02d:%02d",
             dt->year,
             dt->month,
             dt->day,
             dt->hour,
             dt->minute,
             dt->second);
    return true;
}

void DatetimeService::Get(Datetime *dt) {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    struct tm tm = *localtime(&tv.tv_sec);
    dt->second = tm.tm_sec;
    dt->minute = tm.tm_min;
    dt->hour = tm.tm_hour;
    dt->day = tm.tm_mday;
    dt->month = tm.tm_mon + 1;
    dt->year = tm.tm_year + DatetimeService::YearOffset;
}