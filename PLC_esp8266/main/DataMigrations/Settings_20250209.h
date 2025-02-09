#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace MigrateSettings {
    namespace v20250209 {
        inline int GetSizeOfCurrentData();
        inline void MigrateUp(void *pCurr, void *pPrev);
        inline void MigrateDown(void *pCurr, void *pPrev);

        const TDataMigrate DataMigrate = { 0x20250209,
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
                uint32_t reconnect_delay_ms;
                uint32_t scan_station_rssi_period_ms;
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
            auto pPrevSettings = (v20250107::Snapshot::device_settings *)pPrev;

            pCurrSettings->smartconfig.counter = pPrevSettings->smartconfig.counter;

            memcpy(pCurrSettings->wifi_station.ssid,
                   pPrevSettings->wifi_station.ssid,
                   sizeof(pCurrSettings->wifi_station.ssid));
            memcpy(pCurrSettings->wifi_station.password,
                   pPrevSettings->wifi_station.password,
                   sizeof(pCurrSettings->wifi_station.password));
            pCurrSettings->wifi_station.connect_max_retry_count =
                pPrevSettings->wifi_station.connect_max_retry_count;
            pCurrSettings->wifi_station.reconnect_delay_ms = 3000;
            pCurrSettings->wifi_station.scan_station_rssi_period_ms = 5000;

            memcpy(&pCurrSettings->wifi_scanner,
                   &pPrevSettings->wifi_scanner,
                   sizeof(pCurrSettings->wifi_scanner));

            memcpy(&pCurrSettings->wifi_access_point,
                   &pPrevSettings->wifi_access_point,
                   sizeof(pCurrSettings->wifi_access_point));

            ESP_LOGI("Settings_20250209", "Migrate to %08X\n", DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;
            auto pPrevSettings = (v20250107::Snapshot::device_settings *)pPrev;

            pPrevSettings->smartconfig.counter = pCurrSettings->smartconfig.counter;

            memcpy(pPrevSettings->wifi_station.ssid,
                   pCurrSettings->wifi_station.ssid,
                   sizeof(pPrevSettings->wifi_station.ssid));
            memcpy(pPrevSettings->wifi_station.password,
                   pCurrSettings->wifi_station.password,
                   sizeof(pPrevSettings->wifi_station.password));
            pPrevSettings->wifi_station.connect_max_retry_count =
                pCurrSettings->wifi_station.connect_max_retry_count;

            memcpy(&pPrevSettings->wifi_scanner,
                   &pCurrSettings->wifi_scanner,
                   sizeof(pPrevSettings->wifi_scanner));

            memcpy(&pPrevSettings->wifi_access_point,
                   &pCurrSettings->wifi_access_point,
                   sizeof(pPrevSettings->wifi_access_point));
        }

    } // namespace v20250209
} // namespace MigrateSettings
