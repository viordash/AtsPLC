#include "Display/Common.h"
#include "Display/LogsList.h"
#include "Display/display.h"
#include "LogicProgram/Ladder.h"
#include "Maintenance/ServiceModeHandler.h"
#include "Maintenance/backups_storage.h"
#include "buttons.h"
#include "esp_err.h"
#include "esp_log.h"
#include "partitions.h"
#include "redundant_storage.h"
#include "sys_gpio.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG_ServiceModeHandler_Backup = "ServiceMode.Backup";

void ServiceModeHandler::Backup(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler_Backup, "execute");

    int backup_fileno = 0;
    bool files_stat[max_backup_files];
    GetBackupFilesStat(files_stat, max_backup_files);

    ListBox listBox("Backup");

    for (size_t i = 0; i < max_backup_files; i++) {
        char buffer[32];
        CreateBackupName(i, buffer);
        if (files_stat[i]) {
            strcat(buffer, " (stored)");
        }
        listBox.Insert(i, buffer);
    }

    listBox.Select(backup_fileno);

    bool success = false;
    bool error = false;
    while (!success && !error) {
        uint8_t *fb = begin_render();
        listBox.Render(fb);
        end_render(fb);

        EventBits_t uxBits = xEventGroupWaitBits(gpio_events,
                                                 EXPECTED_BUTTONS,
                                                 true,
                                                 false,
                                                 service_mode_timeout_ms / portTICK_PERIOD_MS);

        bool timeout = (uxBits & EXPECTED_BUTTONS) == 0;
        if (timeout) {
            ESP_LOGI(TAG_ServiceModeHandler_Backup, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler_Backup,
                 "buttons_changed, pressed_button:%u, bits:0x%08X",
                 (unsigned int)pressed_button,
                 (unsigned int)uxBits);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
                backup_fileno--;
                if (backup_fileno < 0) {
                    backup_fileno = max_backup_files - 1;
                }
                listBox.Select(backup_fileno);
                break;
            case ButtonsPressType::DOWN_PRESSED:
                backup_fileno++;
                if ((size_t)backup_fileno >= max_backup_files) {
                    backup_fileno = 0;
                }
                listBox.Select(backup_fileno);
                break;
            case ButtonsPressType::SELECT_PRESSED:
                success = CreateBackup(backup_fileno);
                error = !success;
                break;
            default:
                break;
        }
    }
    ShowStatus(gpio_events, success, "Backup completed!", "Backup error!");
}

void ServiceModeHandler::GetBackupFilesStat(bool *files_stat, size_t files_count) {
    backups_storage storage;
    char backup_name[32];
    for (uint32_t i = 0; i < files_count; i++) {
        CreateBackupName(i, backup_name);
        files_stat[i] = backups_storage_load(backup_name, &storage) && storage.size > 0
                     && storage.version == BACKUPS_VERSION;
    }
}

void ServiceModeHandler::CreateBackupName(uint32_t fileno, char *name) {
    sprintf(name, "ladder_%u", (unsigned int)fileno);
}

bool ServiceModeHandler::CreateBackup(uint32_t fileno) {
    char backup_name[32];
    CreateBackupName(fileno, backup_name);

    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       ladder_storage_name);

    if (storage.size == 0) {
        ESP_LOGE(TAG_Ladder, "Cannot read ladder program");
        delete[] storage.data;
        return false;
    }
    if (storage.version != LADDER_VERSION) {
        ESP_LOGE(TAG_Ladder,
                 "Wrong ladder program version,  0x%X<>0x%X",
                 (unsigned int)storage.version,
                 (unsigned int)LADDER_VERSION);
        delete[] storage.data;
        return false;
    }

    ESP_LOGI(TAG_Ladder,
             "Load ver: 0x%X, size:%u, backup:'%s'",
             (unsigned int)storage.version,
             (unsigned int)storage.size,
             backup_name);

    backups_storage backup_storage;
    backup_storage.data = storage.data;
    backup_storage.size = storage.size;
    backup_storage.version = BACKUPS_VERSION;
    backups_storage_store(backup_name, &backup_storage);

    delete[] storage.data;
    return true;
}

void ServiceModeHandler::DeleteBackupFiles(size_t files_count) {
    ESP_LOGI(TAG_ServiceModeHandler_Backup, "delete backup files");
    char backup_name[32];
    for (uint32_t i = 0; i < files_count; i++) {
        CreateBackupName(i, backup_name);
        backups_storage_delete(backup_name);
    }
}