#include "DatetimeService.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "hotreload_service.h"
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

    const TickType_t update_current_time = 60000 / portTICK_PERIOD_MS;
    TickType_t ticks_to_wait = update_current_time;
    Datetime datetime;
    uint32_t ulNotifiedValue = 0;
    while (true) {
        bool timeout =
            xTaskNotifyWait(0, STORE_BIT | RESTART_SNTP_BIT, &ulNotifiedValue, ticks_to_wait)
            == pdFAIL;

        ESP_LOGD(TAG_DatetimeService, "new request, uxBits:0x%08X", (unsigned int)ulNotifiedValue);

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
                         (int)datetime.year,
                         (int)datetime.month,
                         (int)datetime.day,
                         (int)datetime.hour,
                         (int)datetime.minute,
                         (int)datetime.second);
                ticks_to_wait = update_current_time;
            } else {
                ESP_LOGW(TAG_DatetimeService,
                         "Invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
                         (int)datetime.year,
                         (int)datetime.month,
                         (int)datetime.day,
                         (int)datetime.hour,
                         (int)datetime.minute,
                         (int)datetime.second);
                ticks_to_wait = portMAX_DELAY;
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
               || settings.datetime.sntp_server_secondary[0] != 0;
    return enable;
}

void DatetimeService::SntpStateChanged() {
    xTaskNotify(task_handle, RESTART_SNTP_BIT, eNotifyAction::eSetBits);
}

void DatetimeService::StoreSystemDatetime() {
    xTaskNotify(task_handle, STORE_BIT, eNotifyAction::eSetBits);
}

static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG_DatetimeService,
             "sntp_set_time_sync_notification_cb: %u",
             (unsigned int)tv->tv_sec);
    Controller::StoreSystemDatetime();
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
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
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
             "SntpInProcess, serv_0:%u, serv_1:%u",
             (unsigned int)sntp_getreachability(0),
             (unsigned int)sntp_getreachability(1));
    return sntp_getreachability(0) != 0 || sntp_getreachability(1) != 0;
}

void DatetimeService::GetCurrent(timeval *tv) {
    ESP_ERROR_CHECK(gettimeofday(tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
}

void DatetimeService::SetCurrent(const timeval *tv) {
    ESP_ERROR_CHECK(settimeofday(tv, NULL) == 0 ? ESP_OK : ESP_FAIL);
}

int DatetimeService::GetCurrentSecond() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentSecond: %d", (int)tm.tm_sec);
    return tm.tm_sec;
}

int DatetimeService::GetCurrentMinute() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMinute: %d", (int)tm.tm_min);
    return tm.tm_min;
}

int DatetimeService::GetCurrentHour() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentHour: %d", (int)tm.tm_hour);
    return tm.tm_hour;
}

int DatetimeService::GetCurrentDay() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentDay: %d", (int)tm.tm_mday);
    return tm.tm_mday;
}

int DatetimeService::GetCurrentWeekday() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentWeekday: %d", (int)tm.tm_wday + 1);
    return tm.tm_wday + 1;
}

int DatetimeService::GetCurrentMonth() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentMonth: %d", (int)tm.tm_mon + 1);
    return tm.tm_mon + 1;
}

int DatetimeService::GetCurrentYear() {
    timeval tv;
    GetCurrent(&tv);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    ESP_LOGD(TAG_DatetimeService, "GetCurrentYear: %d", (int)tm.tm_year);
    return tm.tm_year;
}

bool DatetimeService::ManualSet(Datetime *dt) {
    if (!ValidateDatetime(dt)) {
        ESP_LOGW(TAG_DatetimeService,
                 "ManualSet, invalid datetime: %04d-%02d-%02d %02d:%02d:%02d",
                 (int)dt->year,
                 (int)dt->month,
                 (int)dt->day,
                 (int)dt->hour,
                 (int)dt->minute,
                 (int)dt->second);
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
    SetCurrent(&new_tv);

    xTaskNotify(task_handle, STORE_BIT, eNotifyAction::eSetBits);

    ESP_LOGI(TAG_DatetimeService,
             "ManualSet: %04d-%02d-%02d %02d:%02d:%02d",
             (int)dt->year,
             (int)dt->month,
             (int)dt->day,
             (int)dt->hour,
             (int)dt->minute,
             (int)dt->second);
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