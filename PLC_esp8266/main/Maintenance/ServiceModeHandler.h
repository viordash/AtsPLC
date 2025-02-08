#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#define EXPECTED_BUTTONS                                                                           \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

#define BACKUPS_VERSION ((uint32_t)0x20250202)

class ServiceModeHandler {
  public:
    enum Mode {
        sm_SmartConfig = 0,
        sm_BackupLogic = 1,
        sm_RestoreLogic = 2,
        sm_ResetToDefault = 3
    };

    enum ResetMode { rd_Settings = 0, rd_Ladder = 1, rd_Backups = 2, rd_FactoryReset = 3 };

  protected:
    static const int service_mode_timeout_ms = 120000;
    static const size_t max_backup_files = 4;
    static const char *reset_data_names[];

    static Mode ChangeModeToPrev(Mode mode);
    static Mode ChangeModeToNext(Mode mode);
    static void Execute(EventGroupHandle_t gpio_events, Mode mode);
    static void SmartConfig(EventGroupHandle_t gpio_events);

    static void Backup(EventGroupHandle_t gpio_events);
    static void GetBackupFilesStat(bool *files_stat, size_t files_count);
    static void CreateBackupName(uint32_t fileno, char *name);
    static bool CreateBackup(uint32_t fileno);
    static void DeleteBackupFiles(size_t files_count);

    static void Restore(EventGroupHandle_t gpio_events);
    static bool DoRestore(uint32_t fileno);

    static void ResetData(EventGroupHandle_t gpio_events);
    static ResetMode ChangeResetModeToPrev(ResetMode mode);
    static ResetMode ChangeResetModeToNext(ResetMode mode);
    static void DoResetData(EventGroupHandle_t gpio_events, ResetMode mode);

    static void ShowStatus(EventGroupHandle_t gpio_events,
                           bool success,
                           const char *success_message,
                           const char *error_message);

  public:
    static void Start(EventGroupHandle_t gpio_events);
};
