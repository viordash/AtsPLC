#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace MigrateSettings {
    namespace v20250619 {
        inline int GetSizeOfCurrentData();
        inline void MigrateUp(void *pCurr, void *pPrev);
        inline void MigrateDown(void *pCurr, void *pPrev);

        const TDataMigrate DataMigrate = { 0x20250619,
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
                int8_t max_rssi;
                int8_t min_rssi;
                uint32_t min_worktime_ms;
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
                char sntp_server_primary[64];
                char sntp_server_secondary[64];
                char timezone[32];
            } datetime_settings;

            typedef struct {
                uint32_t scan_period_ms;
            } adc_settings;

            typedef struct {
                smartconfig_settings smartconfig;
                wifi_station_settings wifi_station;
                wifi_scanner_settings wifi_scanner;
                wifi_access_point_settings wifi_access_point;
                datetime_settings datetime;
                adc_settings adc;
            } device_settings;
        } // namespace Snapshot

        inline int GetSizeOfCurrentData() {
            return sizeof(Snapshot::device_settings);
        }

        inline void MigrateUp(void *pCurr, void *pPrev) {
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;
            auto pPrevSettings = (v20250413::Snapshot::device_settings *)pPrev;

            memcpy(&pCurrSettings->smartconfig,
                   &pPrevSettings->smartconfig,
                   sizeof(pPrevSettings->smartconfig));
            memcpy(&pCurrSettings->wifi_station,
                   &pPrevSettings->wifi_station,
                   sizeof(pCurrSettings->wifi_station));
            memcpy(&pCurrSettings->wifi_scanner,
                   &pPrevSettings->wifi_scanner,
                   sizeof(pCurrSettings->wifi_scanner));
            memcpy(&pCurrSettings->wifi_access_point,
                   &pPrevSettings->wifi_access_point,
                   sizeof(pCurrSettings->wifi_access_point));
            memcpy(&pCurrSettings->datetime,
                   &pPrevSettings->datetime,
                   sizeof(pCurrSettings->datetime));

            pCurrSettings->adc.scan_period_ms = 1000;

            ESP_LOGI("Settings_20250619", "Migrate to %08X", (unsigned int)DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            auto pCurrSettings = (Snapshot::device_settings *)pCurr;
            auto pPrevSettings = (v20250413::Snapshot::device_settings *)pPrev;

            memcpy(&pPrevSettings->smartconfig,
                   &pCurrSettings->smartconfig,
                   sizeof(pPrevSettings->smartconfig));
            memcpy(&pPrevSettings->wifi_station,
                   &pCurrSettings->wifi_station,
                   sizeof(pPrevSettings->wifi_station));
            memcpy(&pPrevSettings->wifi_scanner,
                   &pCurrSettings->wifi_scanner,
                   sizeof(pPrevSettings->wifi_scanner));
            memcpy(&pPrevSettings->wifi_access_point,
                   &pCurrSettings->wifi_access_point,
                   sizeof(pPrevSettings->wifi_access_point));
            memcpy(&pPrevSettings->datetime,
                   &pCurrSettings->datetime,
                   sizeof(pPrevSettings->datetime));
        }

    } // namespace v20250619
} // namespace MigrateSettings
