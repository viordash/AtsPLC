#include <stdint.h>
#include <unistd.h>

typedef struct {
    uint8_t *data;
    size_t size;
} redundant_storage;

redundant_storage redundant_storage_load(const char *partition_0,
                                         const char *path_0,
                                         const char *partition_1,
                                         const char *path_1);

void redundant_storage_store(const char *partition_0,
                             const char *path_0,
                             const char *partition_1,
                             const char *path_1,
                             redundant_storage storage);