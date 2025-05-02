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

static const char *TAG_ServiceModeHandler_Restore = "ServiceMode.Restore";

void ServiceModeHandler::Restore(EventGroupHandle_t gpio_events) {
    ESP_LOGI(TAG_ServiceModeHandler_Restore, "execute");

    int backup_fileno = 0;
    bool files_stat[max_backup_files];
    GetBackupFilesStat(files_stat, max_backup_files);

    ListBox listBox("Restore");

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
            ESP_LOGI(TAG_ServiceModeHandler_Restore, "timeout, returns to main");
            return;
        }

        ButtonsPressType pressed_button = handle_buttons(uxBits);
        ESP_LOGD(TAG_ServiceModeHandler_Restore,
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
                if (files_stat[backup_fileno]) {
                    success = DoRestore(backup_fileno);
                    error = !success;
                } else {
                    error = true;
                }
                break;
            default:
                break;
        }
    }
    ShowStatus(gpio_events, success, "Restore completed!", "Restore error!");
}

bool ServiceModeHandler::DoRestore(uint32_t fileno) {
    char backup_name[16];
    CreateBackupName(fileno, backup_name);

    backups_storage backup_storage;
    if (!backups_storage_load(backup_name, &backup_storage)) {
        ESP_LOGE(TAG_Ladder, "Cannot restore '%s'", backup_name);
        return false;
    }
    if (backup_storage.size == 0) {
        ESP_LOGE(TAG_Ladder, "Empty backup '%s'", backup_name);
        delete[] backup_storage.data;
        return false;
    }
    if (backup_storage.version != BACKUPS_VERSION) {
        ESP_LOGE(TAG_Ladder,
                 "Wrong backup '%s' version,  0x%X<>0x%X",
                 backup_name,
                 (unsigned int)backup_storage.version,
                 (unsigned int)BACKUPS_VERSION);
        delete[] backup_storage.data;
        return false;
    }

    redundant_storage storage;
    storage.size = backup_storage.size;
    storage.data = backup_storage.data;
    storage.version = LADDER_VERSION;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);

    ESP_LOGI(TAG_Ladder,
             "Store ver: 0x%X, size:%u, backup:'%s'",
             (unsigned int)storage.version,
             (unsigned int)storage.size,
             backup_name);

    delete[] backup_storage.data;
    return true;
}