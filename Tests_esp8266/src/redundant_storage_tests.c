#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "redundant_storage.c"
#include "storage.c"

static const char *storage_0_partition = "storage_0";
static const char *storage_1_partition = "storage_1";
static const char *storage_0_path = "/storage_0";
static const char *storage_1_path = "/storage_1";

TEST_C(RedundantStorageTestsGroup, load_if_clear_storage_return_NULL) {
    mock_scope_c("storage_0")->expectOneCall("esp_vfs_spiffs_register")->ignoreOtherParameters();
    mock_scope_c("storage_1")->expectOneCall("esp_vfs_spiffs_register")->ignoreOtherParameters();
    mock_scope_c("storage_0")->expectOneCall( "esp_vfs_spiffs_unregister")->ignoreOtherParameters();
    mock_scope_c("storage_1")->expectOneCall( "esp_vfs_spiffs_unregister")->ignoreOtherParameters();

    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path);
    CHECK_EQUAL_C_POINTER(NULL, storage.data);
    CHECK_EQUAL_C_UINT(0, storage.size);
}