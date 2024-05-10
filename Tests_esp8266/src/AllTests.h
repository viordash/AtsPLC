
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

TEST_GROUP_C_WRAPPER(Crc32TestsGroup){};
TEST_C_WRAPPER(Crc32TestsGroup, data_32);
TEST_C_WRAPPER(Crc32TestsGroup, data_16);
TEST_C_WRAPPER(Crc32TestsGroup, empty_data);
TEST_C_WRAPPER(Crc32TestsGroup, null_data);


TEST_GROUP_C_WRAPPER(HotReloadTestsGroup){};
TEST_C_WRAPPER(HotReloadTestsGroup, store_load);