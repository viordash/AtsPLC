#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Maintenance/backups_storage.h"
#include "main/storage.h"

#include "tests_utils.h"

TEST_GROUP(BackupsStorageTestsGroup){ //
                                      TEST_SETUP(){ create_backups_storage();
}

TEST_TEARDOWN() {
    remove_backups_storage();
}
}
;

TEST(BackupsStorageTestsGroup, load_if_clear_storage_return_NULL) {
    mock("backups").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("backups").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    backups_storage storage;
    CHECK_FALSE(backups_storage_load(backups_storage_name, &storage));
}

TEST(BackupsStorageTestsGroup, store) {
    mock("backups").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("backups").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    backups_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    backups_storage_store(backups_storage_name, &storage);
}

TEST(BackupsStorageTestsGroup, load) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    backups_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    mock().disable();
    backups_storage_store(backups_storage_name, &storage);
    mock().enable();

    mock("backups").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("backups").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    backups_storage load_storage;
    CHECK_TRUE(backups_storage_load(backups_storage_name, &load_storage));

    MEMCMP_EQUAL(data, load_storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), load_storage.size);
    CHECK_EQUAL(42, load_storage.version);
    delete[] load_storage.data;
}

TEST(BackupsStorageTestsGroup, delete_storage) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    backups_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    mock().disable();
    backups_storage_store(backups_storage_name, &storage);
    mock().enable();

    mock("backups").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("backups").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    CHECK_TRUE(backups_storage_exists(backups_storage_name));

    backups_storage_delete(backups_storage_name);

    CHECK_FALSE(backups_storage_exists(backups_storage_name));
}