#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define DEVICE_SETTINGS_VERSION ((uint32_t)0x20250107)

typedef struct {
    uint32_t counter;
} smartconfig_settings;

typedef struct {
    char ssid[32];
    char password[64];
    int32_t connect_max_retry_count;
} wifi_station_settings;

typedef struct {
    uint16_t per_channel_scan_time_ms;
    int8_t max_rssi;
    int8_t min_rssi;
} wifi_scanner_settings;

typedef struct {
    uint32_t generation_time_ms;
    bool ssid_hidden;
} wifi_access_point_settings;

typedef struct {
    smartconfig_settings smartconfig;
    wifi_station_settings wifi_station;
    wifi_scanner_settings wifi_scanner;
    wifi_access_point_settings wifi_access_point;
} device_settings;

extern device_settings settings;

#ifdef __cplusplus
extern "C" {
#endif

void load_settings();
void store_settings();

void lock_settings();
void unlock_settings();

#define SAFETY_SETTINGS(action)                                                                    \
    {                                                                                              \
        lock_settings();                                                                           \
        action;                                                                                    \
        unlock_settings();                                                                         \
    }

#ifdef __cplusplus
}
#endif