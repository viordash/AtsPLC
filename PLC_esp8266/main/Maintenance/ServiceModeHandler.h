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
    enum Mode { sm_SmartConfig, sm_BackupLogic, sm_RestoreLogic, sm_ResetToDefault };

  protected:
    static const int service_mode_timeout_ms = 120000;
    static const size_t max_backup_files = 4;

    static void RenderMainMenu(Mode mode);
    static Mode ChangeModeToPrev(Mode mode);
    static Mode ChangeModeToNext(Mode mode);
    static void Execute(EventGroupHandle_t gpio_events, Mode mode);
    static void SmartConfig(EventGroupHandle_t gpio_events);

    static void Backup(EventGroupHandle_t gpio_events);
    static void RenderBackup(uint32_t fileno, bool *files_stat, size_t files_count);
    static void GetBackupFilesStat(bool *files_stat, size_t files_count);
    static void CreateBackupName(uint32_t fileno, char * name);
    static void CreateBackup(uint32_t fileno);

  public:
    static void Start(EventGroupHandle_t gpio_events);
};
