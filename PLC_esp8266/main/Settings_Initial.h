#pragma once

#include "MigrateAnyData.h"
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
                size_t count;
                uint32_t state;
            } device_settings;
        } // namespace Snapshot

        inline int GetSizeOfCurrentData() {
            return sizeof(Snapshot::device_settings);
        }

        inline void MigrateUp(void *pCurr, void *pPrev) {
            (void)pPrev;
            auto pSettings = (Snapshot::device_settings *)pCurr;
            memset(pSettings, 0, sizeof(*pSettings));
            pSettings->count = 1;
            pSettings->state = 0xFF;

            ESP_LOGI("Settings_Initial",
                     "Settings. Initial migrate to %04X\n",
                     DataMigrate.Version);
        }

        inline void MigrateDown(void *pCurr, void *pPrev) {
            (void)pCurr;
            (void)pPrev;
        }

    } // namespace Initial
} // namespace MigrateSettings
