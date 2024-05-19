#pragma once

#include <stdint.h>
#include <unistd.h>

#define CRC32_INIT ~0U

#ifdef __cplusplus
extern "C" {
#endif

uint32_t calc_crc32(uint32_t crc, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif