#include "DatetimeService.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "settings.h"
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
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentSecond: %d", tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentMinute: %d", tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentHour: %d", tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentDay: %d", tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentWeekday: %d", tm.tm_wday + 1);
    return tm.tm_wday + 1;
}

int DatetimeService::GetCurrentMonth() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentMonth: %d", tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    ESP_LOGI(TAG_DatetimeService, "GetCurrentYear: %d", tm.tm_year);
    return tm.tm_year;
}
