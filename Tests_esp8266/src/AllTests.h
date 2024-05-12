
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

TEST_GROUP_C_WRAPPER(Crc32TestsGroup){};
TEST_C_WRAPPER(Crc32TestsGroup, data_32);
TEST_C_WRAPPER(Crc32TestsGroup, data_16);
TEST_C_WRAPPER(Crc32TestsGroup, empty_data);
TEST_C_WRAPPER(Crc32TestsGroup, null_data);

TEST_GROUP_C_WRAPPER(HotReloadTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(HotReloadTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(HotReloadTestsGroup);
};
TEST_C_WRAPPER(HotReloadTestsGroup, store_load);
TEST_C_WRAPPER(HotReloadTestsGroup, load_if_memory_cleared);

TEST_GROUP_C_WRAPPER(GpioTestsGroup){};
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_cleared_startup_value);
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_startup_for_out0);
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_startup_for_out1);

TEST_GROUP_C_WRAPPER(RedundantStorageTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(RedundantStorageTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(RedundantStorageTestsGroup);
};
TEST_C_WRAPPER(RedundantStorageTestsGroup, load_if_clear_storage_return_NULL);
TEST_C_WRAPPER(RedundantStorageTestsGroup, store);