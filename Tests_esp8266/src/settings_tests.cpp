#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "MigrateAnyData.cpp"
#include "settings.cpp"

#undef storage_0_partition
#undef storage_1_partition
#undef storage_0_path
#undef storage_1_path
#undef storage_name

#define storage_0_partition "storage_0"
#define storage_1_partition "storage_1"
#define storage_0_path "/tmp/storage_0"
#define storage_1_path "/tmp/storage_1"
#define storage_name "settings"

static void create_storage() {
    mkdir(storage_0_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(storage_1_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

static void remove_storage() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", storage_0_path);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "rm -rf %s", storage_1_path);
    system(cmd);
}

TEST_GROUP(SettingsTestsGroup){ //
                                TEST_SETUP(){ create_storage();
// mock().disable();
}

TEST_TEARDOWN() {
    remove_storage();
    // mock().enable();
}
}
;

TEST(SettingsTestsGroup, load_if_clear_storage_return_default_settings) {
    redundant_storage storage = {};
    mock()
        .expectOneCall("redundant_storage_load")
        .withOutputParameterReturning("storage", &storage, sizeof(storage))
        .ignoreOtherParameters();

    settings.count = 42;
    settings.state = 19;
    load_settings();

    CHECK_EQUAL(1, settings.count);
    CHECK_EQUAL(0xFF, settings.state);
}

TEST(SettingsTestsGroup, load_settings_without_migration) {
    device_settings *stored_settings = (device_settings *)malloc(sizeof(device_settings));
    stored_settings->count = 42;
    stored_settings->state = 1;
    redundant_storage storage;
    storage.data = (uint8_t *)stored_settings;
    storage.size = sizeof(stored_settings);
    storage.version = DEVICE_SETTINGS_VERSION;

    mock()
        .expectOneCall("redundant_storage_load")
        .withStringParameter("partition_0", "storage_0")
        .withStringParameter("path_0", "/storage_0")
        .withStringParameter("partition_1", "storage_1")
        .withStringParameter("path_1", "/storage_1")
        .withStringParameter("name", "settings")
        .withOutputParameterReturning("storage", &storage, sizeof(storage));

    settings.count = 0;
    settings.state = 0;
    load_settings();

    CHECK_EQUAL(42, settings.count);
    CHECK_EQUAL(1, settings.state);
}

TEST(SettingsTestsGroup, load_settings_and_migrate) {
    device_settings *stored_settings = (device_settings *)malloc(sizeof(device_settings));
    stored_settings->count = 42;
    stored_settings->state = 1;
    redundant_storage storage;
    storage.data = (uint8_t *)stored_settings;
    storage.size = sizeof(stored_settings);
    storage.version = 0;

    mock()
        .expectOneCall("redundant_storage_load")
        .withStringParameter("partition_0", "storage_0")
        .withStringParameter("path_0", "/storage_0")
        .withStringParameter("partition_1", "storage_1")
        .withStringParameter("path_1", "/storage_1")
        .withStringParameter("name", "settings")
        .withOutputParameterReturning("storage", &storage, sizeof(storage));

    settings.count = 0;
    settings.state = 0;
    load_settings();

    CHECK_EQUAL(1, settings.count);
    CHECK_EQUAL(0xFF, settings.state);
}

TEST(SettingsTestsGroup, store_settings) {
    mock()
        .expectOneCall("redundant_storage_store")
        .withStringParameter("partition_0", "storage_0")
        .withStringParameter("path_0", "/storage_0")
        .withStringParameter("partition_1", "storage_1")
        .withStringParameter("path_1", "/storage_1")
        .withStringParameter("name", "settings")
        .withPointerParameter("storage.data", (uint8_t *)&settings)
        .withUnsignedIntParameter("storage.size", sizeof(settings))
        .withUnsignedIntParameter("storage.version", DEVICE_SETTINGS_VERSION);

    store_settings();
}

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1,
                                         const char *name) {

    redundant_storage storage;
    mock()
        .actualCall("redundant_storage_load")
        .withStringParameter("partition_0", partition_0)
        .withStringParameter("path_0", path_0)
        .withStringParameter("partition_1", partition_1)
        .withStringParameter("path_1", path_1)
        .withStringParameter("name", name)
        .withOutputParameter("storage", &storage);
    return storage;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage storage) {

    mock()
        .actualCall("redundant_storage_store")
        .withStringParameter("partition_0", partition_0)
        .withStringParameter("path_0", path_0)
        .withStringParameter("partition_1", partition_1)
        .withStringParameter("path_1", path_1)
        .withStringParameter("name", name)
        .withPointerParameter("storage.data", storage.data)
        .withUnsignedIntParameter("storage.size", storage.size)
        .withUnsignedIntParameter("storage.version", storage.version);
}