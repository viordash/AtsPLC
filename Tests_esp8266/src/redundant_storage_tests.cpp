#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/redundant_storage.h"
#include "main/storage.h"

#include "tests_utils.h"

TEST_GROUP(RedundantStorageTestsGroup){ //
                                        TEST_SETUP(){ create_storage_0();
create_storage_1();
}

TEST_TEARDOWN() {
    remove_storage_0();
    remove_storage_1();
}
}
;

TEST(RedundantStorageTestsGroup, load_if_clear_storage_return_NULL) {
    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       settings_storage_name);
    POINTERS_EQUAL(NULL, storage.data);
    CHECK_EQUAL(0, storage.size);
    CHECK_EQUAL(0, storage.version);
}

TEST(RedundantStorageTestsGroup, store) {
    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
}

TEST(RedundantStorageTestsGroup, load) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    mock().disable();
    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
    mock().enable();

    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    storage = redundant_storage_load(storage_0_partition,
                                     storage_0_path,
                                     storage_1_partition,
                                     storage_1_path,
                                     settings_storage_name);
    MEMCMP_EQUAL(data, storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), storage.size);
    CHECK_EQUAL(42, storage.version);
    delete[] storage.data;
}

TEST(RedundantStorageTestsGroup, second_storage_restored_when_load) {
    uint8_t data[] = { 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 19;

    mock().disable();
    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
    mock().enable();

    remove_storage_1();
    create_storage_1();
    CHECK_EQUAL(true, storage_0_exists(settings_storage_name));
    CHECK_EQUAL(false, storage_1_exists(settings_storage_name));

    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    storage = redundant_storage_load(storage_0_partition,
                                     storage_0_path,
                                     storage_1_partition,
                                     storage_1_path,
                                     settings_storage_name);
    MEMCMP_EQUAL(data, storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), storage.size);
    CHECK_EQUAL(19, storage.version);
    delete[] storage.data;

    CHECK_EQUAL(true, storage_0_exists(settings_storage_name));
    CHECK_EQUAL(true, storage_1_exists(settings_storage_name));
}

TEST(RedundantStorageTestsGroup, first_storage_restored_when_load) {
    uint8_t data[] = { 0xee, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    mock().disable();
    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
    mock().enable();

    remove_storage_0();
    create_storage_0();
    CHECK_EQUAL(false, storage_0_exists(settings_storage_name));
    CHECK_EQUAL(true, storage_1_exists(settings_storage_name));

    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    storage = redundant_storage_load(storage_0_partition,
                                     storage_0_path,
                                     storage_1_partition,
                                     storage_1_path,
                                     settings_storage_name);
    MEMCMP_EQUAL(data, storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), storage.size);
    CHECK_EQUAL(42, storage.version);
    delete[] storage.data;

    CHECK_EQUAL(true, storage_0_exists(settings_storage_name));
    CHECK_EQUAL(true, storage_1_exists(settings_storage_name));
}

TEST(RedundantStorageTestsGroup, delete_storage) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = 42;

    mock().disable();
    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            settings_storage_name,
                            &storage);
    mock().enable();

    mock("storage_0").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_register").ignoreOtherParameters();
    mock("storage_0").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();
    mock("storage_1").expectOneCall("esp_vfs_spiffs_unregister").ignoreOtherParameters();

    CHECK_TRUE(storage_0_exists(settings_storage_name));
    CHECK_TRUE(storage_1_exists(settings_storage_name));

    redundant_storage_delete(storage_0_partition,
                             storage_0_path,
                             storage_1_partition,
                             storage_1_path,
                             settings_storage_name);

    CHECK_FALSE(storage_0_exists(settings_storage_name));
    CHECK_FALSE(storage_1_exists(settings_storage_name));
}