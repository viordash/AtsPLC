#include "DatetimeService.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
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

void DatetimeService::Set(CurrentSettings::datetime_settings *datetime) {
    timeval tv;
    ESP_ERROR_CHECK(gettimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    struct tm tm = *localtime(&tv.tv_sec);
    tm.tm_sec = datetime->second;
    tm.tm_min = datetime->minute;
    tm.tm_hour = datetime->hour;
    tm.tm_mday = datetime->day;
    tm.tm_mon = datetime->month - 1;
    tm.tm_year = datetime->year;

    timeval new_tv = { mktime(&tm), tv.tv_usec };
    ESP_ERROR_CHECK(settimeofday(&new_tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    ESP_LOGI(TAG_DatetimeService,
             "Set: %04d-%02d-%02d %02d:%02d:%02d",
             datetime->year + DatetimeService::YearOffset,
             datetime->month,
             datetime->day,
             datetime->hour,
             datetime->minute,
             datetime->second);
}