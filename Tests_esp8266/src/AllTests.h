
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

TEST_GROUP_C_WRAPPER(Crc32TestsGroup){};

TEST_C_WRAPPER(Crc32TestsGroup, calc_crc32);