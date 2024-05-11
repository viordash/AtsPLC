#include <stdint.h>
#include <unistd.h>

uint8_t *
open_redundant_storage(const char *path_0, const char *path_1, size_t size, uint32_t version);
void store_redundant_storage(const char *path_0, const char *path_1, uint8_t *data);
void close_redundant_storage(uint8_t *data);