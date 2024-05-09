#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "crc32.c"

//uint32_t calc_crc32(uint32_t crc, const void *buf, size_t size)

TEST_C(Crc32TestsGroup, calc_crc32) {

    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    uint32_t crc = calc_crc32(CRC32_INIT, data, sizeof(data));
    CHECK_EQUAL_C_UINT(0xCECEE288, crc);
}