#include "CppUTest/TestHarness.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "main/crc32.cpp"

TEST_GROUP(Crc32TestsGroup){ //
                             TEST_SETUP(){}

                             TEST_TEARDOWN(){}
};

TEST(Crc32TestsGroup, data_32) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    uint32_t crc = calc_crc32(CRC32_INIT, data, sizeof(data));
    CHECK_EQUAL(0x91267E8A, crc);
}

TEST(Crc32TestsGroup, data_16) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    uint32_t crc = calc_crc32(CRC32_INIT, data, sizeof(data));
    CHECK_EQUAL(0xCECEE288, crc);
}

TEST(Crc32TestsGroup, empty_data) {
    uint8_t data[] = {};
    uint32_t crc = calc_crc32(CRC32_INIT, data, sizeof(data));
    CHECK_EQUAL(0, crc);
}

TEST(Crc32TestsGroup, null_data) {
    uint32_t crc = calc_crc32(CRC32_INIT, NULL, 0);
    CHECK_EQUAL(0, crc);
}