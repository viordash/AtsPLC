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

    while (true) {
        RenderBackup(backup_fileno, files_stat, max_backup_files);

        EventBits_t uxBits = xEventGroupWaitBits(gpio_events,
                                                 EXPECTED_BUTTONS,
                                                 true,
                                                 false,
                                                 service_mode_timeout_ms / portTICK_PERIOD_MS);

        ESP_LOGI(TAG_ServiceModeHandler_Backup, "bits:0x%08X", uxBits);

        bool timeout = (uxBits & EXPECTED_BUTTONS) == 0;
        if (timeout) {
            ESP_LOGI(TAG_ServiceModeHandler_Backup, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGI(TAG_ServiceModeHandler_Backup,
                 "buttons_changed, pressed_button:%u",
                 pressed_button);
        switch (pressed_button) {
            case ButtonsPressType::UP_PRESSED:
                backup_fileno--;
                if (backup_fileno < 0) {
                    backup_fileno = max_backup_files - 1;
                }
                break;
            case ButtonsPressType::DOWN_PRESSED:
                backup_fileno++;
                if (backup_fileno >= max_backup_files) {
                    backup_fileno = 0;
                }
                break;
            case ButtonsPressType::SELECT_PRESSED:
                CreateBackup(backup_fileno);
                return;
            default:
                break;
        }
    }
}

void ServiceModeHandler::RenderBackup(uint32_t fileno, bool *files_stat, size_t files_count) {
    // char buffer[64];
    // uint8_t x = 1;
    // uint8_t y = 1;
    // uint8_t height = get_text_f6X12_height();
    // uint8_t *fb = begin_render();

    // ESP_ERROR_CHECK(draw_text_f5X7(fb, x, y, buffer) <= 0);

    // uint8_t marker_x = x;
    // uint8_t marker_y = y;
    // switch (fileno) {
    //     case Mode::sm_SmartConfig:
    //         marker_y += height * 1;
    //         break;
    //     case Mode::sm_BackupLogic:
    //         marker_y += height * 2;
    //         break;
    //     case Mode::sm_RestoreLogic:
    //         marker_y += height * 3;
    //         break;
    //     case Mode::sm_ResetToDefault:
    //         marker_y += height * 4;
    //         break;
    // }
    // int marker_width = draw_text_f6X12(fb, marker_x, marker_y, "\x01");
    // ESP_ERROR_CHECK(marker_width <= 0);
    // x += marker_width;

    // ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 1, "  Smart config") <= 0);
    // ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 2, "  Backup logic") <= 0);
    // ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 3, "  Restore logic") <= 0);
    // ESP_ERROR_CHECK(draw_text_f6X12(fb, x, y + height * 4, "  Reset to default") <= 0);

    // end_render(fb);
}

void ServiceModeHandler::GetBackupFilesStat(bool *files_stat, size_t files_count) {
    backups_storage storage;
    char backup_name[16];
    for (uint32_t i = 0; i < files_count; i++) {
        CreateBackupName(i, backup_name);
        files_stat[i] = backups_storage_load(backup_name, &storage) && storage.size > 0
                     && storage.version == BACKUPS_VERSION;
    }
}

void ServiceModeHandler::CreateBackupName(uint32_t fileno, char *name) {
    sprintf(name, "backup_%u", fileno);
}

void ServiceModeHandler::CreateBackup(uint32_t fileno) {
    char backup_name[16];
    CreateBackupName(fileno, backup_name);

    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       ladder_storage_name);

    ESP_LOGI(TAG_Ladder, "Load ver: 0x%X, size:%u", storage.version, (uint32_t)storage.size);

    if (storage.version == LADDER_VERSION && storage.size > 0) {
        backups_storage backup_storage;
        backup_storage.data = storage.data;
        backup_storage.size = storage.size;
        backup_storage.version = BACKUPS_VERSION;
        backups_storage_store(backup_name, &backup_storage);
    }
    delete[] storage.data;
}