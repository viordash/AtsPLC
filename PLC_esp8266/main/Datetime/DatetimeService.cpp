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
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentSecond: %d", tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMinute: %d", tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentHour: %d", tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentDay: %d", tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentWeekday: %d", tm.tm_wday + 1);
    return tm.tm_wday + 1;
}

int DatetimeService::GetCurrentMonth() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMonth: %d", tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentYear: %d", tm.tm_year);
    return tm.tm_year;
}

void DatetimeService::SetCurrentSecond(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);

    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_sec = val;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrentMinute(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_min = val;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrentHour(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_hour = val;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrentDay(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_mday = val;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrentMonth(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_mon = val - 1;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrentYear(int val) {
    timespec ts;
    ESP_ERROR_CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 ? ESP_OK : ESP_FAIL);
    struct tm tm = *localtime(&ts.tv_sec);
    tm.tm_year = val;
    struct timespec new_ts = { mktime(&tm), ts.tv_nsec };
    ESP_ERROR_CHECK(clock_settime(CLOCK_REALTIME, &new_ts) == 0 ? ESP_OK : ESP_FAIL);
}
