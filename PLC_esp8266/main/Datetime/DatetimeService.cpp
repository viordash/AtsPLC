#include "DatetimeService.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "hotreload_service.h"
#include "lwip/apps/sntp.h"
#include "settings.h"
#include "sys/time.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char *TAG_DatetimeService = "DatetimeService";
extern CurrentSettings::device_settings settings;

DatetimeService::DatetimeService() : task_handle(NULL) {
}

DatetimeService::~DatetimeService() {
}

void DatetimeService::Start() {
    ESP_ERROR_CHECK(xTaskCreate(DatetimeService::Task,
                                "datetime_task",
                                2048,
                                this,
                                tskIDLE_PRIORITY,
                                &task_handle)
                            != pdPASS
                        ? ESP_FAIL
                        : ESP_OK);

    ESP_LOGI(TAG_DatetimeService, "Start, task_handle:%p", task_handle);
}

void DatetimeService::Task(void *parm) {
    ESP_LOGI(TAG_DatetimeService, "Start task");
    auto datetime_service = static_cast<DatetimeService *>(parm);

    const TickType_t update_current_time = 60000 / portTICK_RATE_MS;
    TickType_t ticks_to_wait = update_current_time;
    Datetime datetime;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        bool timeout =
            xTaskNotifyWait(0, STORE_BIT | RESTART_SNTP_BIT, &ulNotifiedValue, ticks_to_wait)
            == pdFAIL;

        ESP_LOGD(TAG_DatetimeService, "new request, uxBits:0x%08X", ulNotifiedValue);

        if (timeout || (ulNotifiedValue & STORE_BIT) != 0) {
            datetime_service->Get(&datetime);
            if (ValidateDatetime(&datetime)) {
                SAFETY_HOTRELOAD({
                    hotreload->current_datetime.year = datetime.year;
                    hotreload->current_datetime.month = datetime.month;
                    hotreload->current_datetime.day = datetime.day;
                    hotreload->current_datetime.hour = datetime.hour;
                    hotreload->current_datetime.minute = datetime.minute;
                    hotreload->current_datetime.second = datetime.second;
                    store_hotreload();
                });
                ESP_LOGI(TAG_DatetimeService,
                         "Store datetime: %04d-%02d-%02d %02d:%02d:%02d",
                         datetime.year,
                         datetime.month,
                         datetime.day,
                         datetime.hour,
                         datetime.minute,
                         datetime.second);
                ticks_to_wait = update_current_time;
            } else {
                ESP_LOGW(TAG_DatetimeService,
                         "Invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
                         datetime.year,
                         datetime.month,
                         datetime.day,
                         datetime.hour,
                         datetime.minute,
                         datetime.second);
                // ticks_to_wait = portMAX_DELAY;
            }
        }

        bool use_ntp = datetime_service->EnableSntp();

        if (use_ntp) {
            Controller::ConnectToWiFiStation();
            bool restart_ntp = (ulNotifiedValue & RESTART_SNTP_BIT) != 0;
            if (restart_ntp || !datetime_service->SntpInProcess()) {
                datetime_service->StopSntp();
                datetime_service->StartSntp();
            }
        }
    }

    ESP_LOGW(TAG_DatetimeService, "Finish task");
    vTaskDelete(NULL);
}
bool DatetimeService::EnableSntp() {
    bool enable = settings.datetime.sntp_server_primary[0] != 0
               || settings.datetime.sntp_server_secondary[1] != 0;
    return enable;
}

void DatetimeService::SntpStateChanged() {
    xTaskNotify(task_handle, RESTART_SNTP_BIT, eNotifyAction::eSetBits);
}

void DatetimeService::StartSntp() {
    ESP_LOGI(TAG_DatetimeService,
             "Start SNTP, serv_0:%s, serv_1:%s, tz:%s",
             settings.datetime.sntp_server_primary,
             settings.datetime.sntp_server_secondary,
             settings.datetime.timezone);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    if (strnlen(settings.datetime.sntp_server_primary,
                sizeof(settings.datetime.sntp_server_primary))
        > 0) {
        sntp_setservername(0, settings.datetime.sntp_server_primary);
    }
    if (strnlen(settings.datetime.sntp_server_secondary,
                sizeof(settings.datetime.sntp_server_secondary))
        > 0) {
        sntp_setservername(1, settings.datetime.sntp_server_secondary);
    }
    sntp_init();

    setenv("TZ", settings.datetime.timezone, 1);
    tzset();
}

void DatetimeService::StopSntp() {
    ESP_LOGI(TAG_DatetimeService, "Stop SNTP");
    sntp_stop();
}

bool DatetimeService::SntpInProcess() {
    ESP_LOGI(TAG_DatetimeService,
             "SntpInProcess, serv_0:%d, serv_1:%d",
             sntp_getreachability(0),
             sntp_getreachability(1));
    return sntp_getreachability(0) != 0 || sntp_getreachability(1) != 0;
}

void DatetimeService::GetCurrent(timeval *tv) {
    ESP_ERROR_CHECK(gettimeofday(tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
}

int DatetimeService::GetCurrentSecond() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentSecond: %d", tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMinute: %d", tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentHour: %d", tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentDay: %d", tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentWeekday: %d", tm.tm_wday + 1);
    return tm.tm_wday + 1;
}

int DatetimeService::GetCurrentMonth() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMonth: %d", tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
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

    timeval tv;
    GetCurrent(&tv);

    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    tm.tm_sec = dt->second;
    tm.tm_min = dt->minute;
    tm.tm_hour = dt->hour;
    tm.tm_mday = dt->day;
    tm.tm_mon = dt->month - 1;
    tm.tm_year = dt->year - DatetimeService::YearOffset;

    timeval new_tv = { mktime(&tm), tv.tv_usec };
    ESP_ERROR_CHECK(settimeofday(&new_tv, NULL) == 0 ? ESP_OK : ESP_FAIL);

    xTaskNotify(task_handle, STORE_BIT, eNotifyAction::eSetBits);

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
    GetCurrent(&tv);

    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    dt->second = tm.tm_sec;
    dt->minute = tm.tm_min;
    dt->hour = tm.tm_hour;
    dt->day = tm.tm_mday;
    dt->month = tm.tm_mon + 1;
    dt->year = tm.tm_year + DatetimeService::YearOffset;
}