#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define DEVICE_SETTINGS_VERSION ((uint32_t)0x00000001)

typedef struct {
    uint32_t counter;
} smartconfig_settings;

typedef struct {
    char ssid[32 + 1];
    char password[64 + 1];
} wifi_settings;

typedef struct {
    smartconfig_settings smartconfig;
    wifi_settings wifi;
    uint32_t state;
} device_settings;

extern device_settings settings;

#ifdef __cplusplus
extern "C" {
#endif

void load_settings();
void store_settings();

void lock_settings();
void unlock_settings();

#define SAFE_SETTINGS(action)                                                                      \
    {                                                                                              \
        lock_settings();                                                                           \
        action;                                                                                    \
        unlock_settings();                                                                         \
    }

#ifdef __cplusplus
}
#endif