#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    uint32_t gpio;
} hotreload;

bool try_load_hotreload(hotreload *data);
void store_hotreload(hotreload *data);