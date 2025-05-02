#pragma once

#include "MigrateAnyData/MigrateAnyData.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace MigrateSettings {
    namespace Initial {
        inline int GetSizeOfCurrentData();
        inline void MigrateUp(void *pCurr, void *pPrev);
        inline void MigrateDown(void *pCurr, void *pPrev);

        const TDataMigrate DataMigrate = { 0x00000001,
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
            } wifi_settings;

            typedef struct {
                smartconfig_settings smartconfig;
                wifi_settings wifi;
                uint32_t state;
            } device_settings;
        } // namespace Snapshot

        inline int GetSizeOfCurrentData() {
            return sizeof(Snapshot::device_settings);
        }

        inline void MigrateUp(void *pCurr, void *pPrev) {
            (void)pPrev;
            auto pSettings = (Snapshot::device_settings *)pCurr;

            pSettings->smartconfig.counter = 0;
            memset(pSettings->wifi.ssid, 0, sizeof(pSettings->wifi.ssid));
            memset(pSettings->wifi.password, 0, sizeof(pSettings->wifi.password));
            pSettings->wifi.connect_max_retry_count = -1;
            pSettings->state = 0xFF;

            ESP_LOGI("Settings_Initial",
                     "Settings. Initial migrate to %04X",
                     (unsigned int)DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            (void)pCurr;
            (void)pPrev;
        }

    } // namespace Initial
} // namespace MigrateSettings
