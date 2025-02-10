#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace MigrateSettings {
    namespace v20250107 {
        inline int GetSizeOfCurrentData();
        inline void MigrateUp(void *pCurr, void *pPrev);
        inline void MigrateDown(void *pCurr, void *pPrev);

        const TDataMigrate DataMigrate = { 0x20250107,
                                           MigrateUp,
                                           MigrateDown,
                                           GetSizeOfCurrentData };

        namespace Snapshot {
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
        } // namespace Snapshot

        inline int GetSizeOfCurrentData() {
            return sizeof(Snapshot::device_settings);
        }

        inline void MigrateUp(void *pCurr, void *pPrev) {
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;
            auto pPrevSettings = (Initial::Snapshot::device_settings *)pPrev;

            pCurrSettings->smartconfig.counter = pPrevSettings->smartconfig.counter;

            memcpy(pCurrSettings->wifi_station.ssid,
                   pPrevSettings->wifi.ssid,
                   sizeof(pCurrSettings->wifi_station.ssid));
            memcpy(pCurrSettings->wifi_station.password,
                   pPrevSettings->wifi.password,
                   sizeof(pCurrSettings->wifi_station.password));
            pCurrSettings->wifi_station.connect_max_retry_count =
                pPrevSettings->wifi.connect_max_retry_count;

            pCurrSettings->wifi_scanner.per_channel_scan_time_ms = 500;
            pCurrSettings->wifi_scanner.max_rssi = -26;
            pCurrSettings->wifi_scanner.min_rssi = -120;

            pCurrSettings->wifi_access_point.generation_time_ms = 20000;
            pCurrSettings->wifi_access_point.ssid_hidden = false;

            ESP_LOGI("Settings_20250107", "Migrate to %08X", DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;
            auto pPrevSettings = (Initial::Snapshot::device_settings *)pPrev;

            pPrevSettings->smartconfig.counter = pCurrSettings->smartconfig.counter;

            memcpy(pPrevSettings->wifi.ssid,
                   pCurrSettings->wifi_station.ssid,
                   sizeof(pPrevSettings->wifi.ssid));
            memcpy(pPrevSettings->wifi.password,
                   pCurrSettings->wifi_station.password,
                   sizeof(pPrevSettings->wifi.password));

            pPrevSettings->wifi.connect_max_retry_count =
                pCurrSettings->wifi_station.connect_max_retry_count;
        }

    } // namespace v20250107
} // namespace MigrateSettings
