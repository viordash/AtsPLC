#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

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

TEST(SettingsTestsGroup, load_if_clear_storage_return_NULL_settings) {
    settings = (device_settings *)19;
    load_settings();

    CHECK(settings == NULL);
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
        .withOutputParameterOfType("redundant_storage", "storage", &storage);
    return storage;
}

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             const char *name,
                             redundant_storage storage) {

    mock()
        .actualCall("redundant_storage_load")
        .withStringParameter("partition_0", partition_0)
        .withStringParameter("path_0", path_0)
        .withStringParameter("partition_1", partition_1)
        .withStringParameter("path_1", path_1)
        .withStringParameter("name", name)
        .withPointerParameter("storage.data", storage.data)
        .withUnsignedIntParameter("storage.size", storage.size);
}