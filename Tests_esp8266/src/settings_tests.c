#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport_c.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "settings.c"

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

TEST_GROUP_C_SETUP(SettingsTestsGroup) {
    create_storage();
    mock_c()->disable();
}
TEST_GROUP_C_TEARDOWN(SettingsTestsGroup) {
    remove_storage();
    mock_c()->enable();
}

TEST_C(SettingsTestsGroup, load_if_clear_storage_return_NULL_settings) {
    settings = (device_settings *)19;
    load_settings();

    CHECK_EQUAL_C_POINTER(NULL, settings);
}
